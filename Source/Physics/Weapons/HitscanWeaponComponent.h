// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/PhysicsWeaponComponent.h"
#include "HitscanWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHitscanImpact, class AActor*, impactedActor, FVector, impactPosition, FVector, impactDirection);

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHYSICS_API UHitscanWeaponComponent : public UPhysicsWeaponComponent
{
	GENERATED_BODY()

public:
	virtual void Fire() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Hitscan)
	float m_fRange = 5000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	float m_DebugDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
	float m_ImpactDebugRadius = 10.f;

	UPROPERTY(BlueprintAssignable)
	FHitscanImpact onHitscanImpact;
};
