// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/ProjectileWeaponComponent.h"
#include "PhysicsCharacter.h"
#include "PhysicsProjectile.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"

void UProjectileWeaponComponent::Fire()
{
	Super::Fire();
	if (!Character || !Character->GetWorld() || !m_ProjectileClass || !Character->GetFirstPersonCameraComponent())
	{
		return;
	}

	const FVector Start = Character->GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector Direction = Character->GetFirstPersonCameraComponent()->GetForwardVector();
	const FVector SpawnLocation = Start + Direction * MuzzleOffset.X + Character->GetActorRightVector() * MuzzleOffset.Y + Character->GetActorUpVector() * MuzzleOffset.Z;

	FTransform SpawnTransform(Direction.Rotation(), SpawnLocation);
	FActorSpawnParameters Params;
	Params.Owner = Character;
	Params.Instigator = Character;

	if (APhysicsProjectile* Projectile = Character->GetWorld()->SpawnActor<APhysicsProjectile>(m_ProjectileClass, SpawnTransform, Params))
	{
		Projectile->m_OwnerWeapon = this;
		Projectile->m_DebugImpact = true;
		Projectile->m_DebugImpactDuration = m_DebugDuration;

		if (Projectile->ProjectileMovement)
		{
			const bool bGrenade = m_ProjectileClass->GetName().Contains(TEXT("Grenade"));
			Projectile->ProjectileMovement->ProjectileGravityScale = bGrenade ? 1.f : 0.f;
			Projectile->ProjectileMovement->bShouldBounce = bGrenade;
			Projectile->ProjectileMovement->Velocity = Direction * Projectile->ProjectileMovement->InitialSpeed;
		}
	}
}
