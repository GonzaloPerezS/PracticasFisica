// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsWeaponComponent.generated.h"

class APhysicsCharacter;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PHYSICS_API UPhysicsWeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	UPhysicsWeaponComponent();

	UFUNCTION(BlueprintCallable, Category="Weapon")
	bool AttachWeapon(APhysicsCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual void Fire();

	static void ApplyImpact(AActor* ImpactActor, UPrimitiveComponent* ImpactComponent, const FHitResult& Hit,
		const FVector& Direction, float Damage, float Impulse, AController* InstigatorController, AActor* DamageCauser);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	APhysicsCharacter* Character;
	class USceneComponent* FireOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	float ImpactDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage)
	float ImpactImpulse = 1800.f;
};
