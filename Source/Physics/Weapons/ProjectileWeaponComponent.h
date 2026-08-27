// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/PhysicsWeaponComponent.h"
#include "ProjectileWeaponComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSICS_API UProjectileWeaponComponent : public UPhysicsWeaponComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class APhysicsProjectile> m_ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	float m_DebugDuration = 0.5f;

	virtual void Fire() override;
};
