// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/HitscanWeaponComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsCharacter.h"
#include "PhysicsWeaponComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"

void UHitscanWeaponComponent::Fire()
{
	Super::Fire();
	if (!Character || !Character->GetController() || !Character->GetFirstPersonCameraComponent())
	{
		return;
	}

	const FVector Start = Character->GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector Direction = Character->GetFirstPersonCameraComponent()->GetForwardVector();
	const FVector End = Start + Direction * FMath::Max(1.f, m_fRange);

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(HitscanWeapon), true, Character);
	Params.bReturnPhysicalMaterial = true;
	Params.bTraceComplex = true;

	// PhysicsBody lets the ray hit the individual bodies of a Physics Asset,
	// which is important for enemies. Visibility is used as a fallback for
	// normal world geometry and non-physics actors.
	bool bHit = Character->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_PhysicsBody, Params);

	if (!bHit)
	{
		bHit = Character->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	}

	if (!bHit)
	{
		// No visible beam: only show a red point at the end of the ray.
		DrawDebugPoint(Character->GetWorld(), End, 12.f, FColor::Red, false, m_DebugDuration, 0);
		return;
	}

	AActor* ImpactActor = Hit.GetActor();
	UPrimitiveComponent* ImpactComponent = Hit.GetComponent();

	const bool bCanInteract =
		(ImpactComponent && ImpactComponent->IsSimulatingPhysics()) ||
		ImpactActor && (ImpactActor->IsA<ACharacter>() || ImpactActor->GetClass()->GetName().Contains(TEXT("Target")));

	// The raycast remains invisible. Only the impact point is rendered.
	const FColor ImpactColor = bCanInteract ? FColor::Green : FColor::Red;
	DrawDebugPoint(Character->GetWorld(), Hit.ImpactPoint, 16.f, ImpactColor, false, m_DebugDuration, 0);
	DrawDebugSphere(Character->GetWorld(), Hit.ImpactPoint, m_ImpactDebugRadius, 12, ImpactColor, false, m_DebugDuration, 0, 1.5f);

	if (bCanInteract)
	{
		ApplyImpact(ImpactActor, ImpactComponent, Hit, Direction, ImpactDamage, ImpactImpulse,
			Character->GetController(), Character);
	}

	onHitscanImpact.Broadcast(ImpactActor, Hit.ImpactPoint, Direction);
}
