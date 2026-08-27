// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPhysicsWeaponComponent;

UCLASS(config=Game)
class APhysicsProjectile : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool m_DestroyOnHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	float m_Damage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	float m_ImpactImpulse = 1800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	bool m_RadialExplosion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage, meta = (EditCondition = "m_RadialExplosion"))
	float m_ExplosionRadius = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage, meta = (EditCondition = "m_RadialExplosion"))
	float m_ExplosionImpulse = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	bool m_DebugImpact = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	float m_DebugImpactDuration = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Damage)
	UPhysicsWeaponComponent* m_OwnerWeapon;

public:
	APhysicsProjectile();

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};
