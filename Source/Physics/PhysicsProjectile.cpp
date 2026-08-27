// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Weapons/PhysicsWeaponComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"

APhysicsProjectile::APhysicsProjectile()
{
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(6.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	CollisionComp->SetNotifyRigidBodyCollision(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &APhysicsProjectile::OnHit);
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 3.0f;
}

void APhysicsProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == this || !GetWorld())
	{
		return;
	}

	const bool bIsGrenade = m_RadialExplosion || GetClass()->GetName().Contains(TEXT("Grenade"));
	const FVector Direction = ProjectileMovement && !ProjectileMovement->Velocity.IsNearlyZero()
		? ProjectileMovement->Velocity.GetSafeNormal()
		: GetActorForwardVector();

	if (m_DebugImpact)
	{
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, bIsGrenade ? 18.f : 10.f, 20, FColor::Green, false, m_DebugImpactDuration, 0, 2.f);
		DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 14.f, FColor::Green, false, m_DebugImpactDuration);
	}

	if (bIsGrenade)
	{
		const float Radius = FMath::Max(1.f, m_ExplosionRadius);
		const float ExplosionImpulse = FMath::Max(m_ExplosionImpulse, m_ImpactImpulse);

		UGameplayStatics::ApplyRadialDamageWithFalloff(
			this,
			m_Damage,
			0.f,
			GetActorLocation(),
			Radius * 0.1f,
			Radius,
			1.f,
			UDamageType::StaticClass(),
			TArray<AActor*>(),
			this,
			GetInstigatorController(),
			ECC_Visibility);

		// Visualize the exact explosion radius and a radius raycast marker.
		DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 32, FColor::Purple, false, m_DebugImpactDuration, 0, 2.f);
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + FVector(Radius, 0.f, 0.f), FColor::Purple, false, m_DebugImpactDuration, 0, 4.f);

		TArray<FOverlapResult> Overlaps;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(GrenadeImpulse), false, this);

		if (GetWorld()->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity, ECC_PhysicsBody, Sphere, Params))
		{
			for (const FOverlapResult& Result : Overlaps)
			{
				UPrimitiveComponent* Prim = Result.GetComponent();
				AActor* Actor = Result.GetActor();

				if (!Prim || !Actor || Actor == this)
				{
					continue;
				}

				if (Prim->IsSimulatingPhysics())
				{
					Prim->WakeRigidBody();
					Prim->AddRadialImpulse(GetActorLocation(), Radius, ExplosionImpulse, ERadialImpulseFalloff::RIF_Linear, true);
				}

				FHitResult RadialHit;
				RadialHit.Component = Prim;
				RadialHit.ImpactPoint = Actor->GetActorLocation();
				const FVector RadialDirection = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				UPhysicsWeaponComponent::ApplyImpact(Actor, Prim, RadialHit, RadialDirection, 0.f, ExplosionImpulse, GetInstigatorController(), this);
			}
		}
	}
	else
	{
		UPhysicsWeaponComponent::ApplyImpact(OtherActor, OtherComp, Hit, Direction, m_Damage, m_ImpactImpulse, GetInstigatorController(), this);
	}

	if (m_DestroyOnHit)
	{
		Destroy();
	}
}
