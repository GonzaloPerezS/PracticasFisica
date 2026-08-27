// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsWeaponComponent.h"
#include "PhysicsCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "AIController.h"
#include "BreakableTarget.h"
#include "DrawDebugHelpers.h"

namespace
{
	TMap<TWeakObjectPtr<AActor>, float> EnemyHealth;

	bool IsEnemyActor(AActor* Actor)
	{
		if (!Actor || Actor->IsA<APhysicsCharacter>())
		{
			return false;
		}

		ACharacter* Character = Cast<ACharacter>(Actor);
		if (!Character)
		{
			return false;
		}

		return Actor->GetClass()->GetName().Contains(TEXT("Enemy")) ||
			(Character->GetController() && Character->GetController()->IsA<AAIController>());
	}

	FName ResolveHitBone(USkeletalMeshComponent* SkeletalMesh, const FHitResult& Hit)
	{
		if (!SkeletalMesh)
		{
			return NAME_None;
		}

		if (!Hit.BoneName.IsNone() && SkeletalMesh->GetBoneIndex(Hit.BoneName) != INDEX_NONE)
		{
			return Hit.BoneName;
		}

		FVector ClosestBoneLocation = FVector::ZeroVector;
		return SkeletalMesh->FindClosestBone(Hit.ImpactPoint, &ClosestBoneLocation, 0.f, true);
	}

	bool IsHeadBone(const FName& Bone)
	{
		const FString Name = Bone.ToString();
		return Name.Contains(TEXT("head"), ESearchCase::IgnoreCase);
	}

	void EnableRagdoll(USkeletalMeshComponent* SkeletalMesh)
	{
		if (!SkeletalMesh)
		{
			return;
		}

		SkeletalMesh->SetCollisionProfileName(TEXT("Ragdoll"));
		SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SkeletalMesh->SetNotifyRigidBodyCollision(true);
		SkeletalMesh->SetAllBodiesNotifyRigidBodyCollision(true);
		SkeletalMesh->SetAllBodiesSimulatePhysics(true);
		SkeletalMesh->SetSimulatePhysics(true);
		SkeletalMesh->WakeAllRigidBodies();
	}

	void PreparePhysicalAnimation(AActor* Actor, USkeletalMeshComponent* SkeletalMesh, const FName& Bone)
	{
		UPhysicalAnimationComponent* PhysicalAnimation = Actor->FindComponentByClass<UPhysicalAnimationComponent>();
		if (!PhysicalAnimation)
		{
			PhysicalAnimation = NewObject<UPhysicalAnimationComponent>(Actor, TEXT("ImpactPhysicalAnimation"));
			PhysicalAnimation->RegisterComponent();
		}

		PhysicalAnimation->SetSkeletalMeshComponent(SkeletalMesh);

		FPhysicalAnimationData Data;
		Data.bIsLocalSimulation = false;
		Data.OrientationStrength = 7000.f;
		Data.AngularVelocityStrength = 350.f;
		Data.PositionStrength = 7000.f;
		Data.VelocityStrength = 350.f;
		Data.MaxLinearForce = 150000.f;
		Data.MaxAngularForce = 150000.f;

		PhysicalAnimation->ApplyPhysicalAnimationSettingsBelow(Bone, Data, true);
	}
}

UPhysicsWeaponComponent::UPhysicsWeaponComponent()
{
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}

void UPhysicsWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPhysicsWeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	if (FireAnimation != nullptr)
	{
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void UPhysicsWeaponComponent::ApplyImpact(AActor* ImpactActor, UPrimitiveComponent* ImpactComponent, const FHitResult& Hit,
	const FVector& Direction, float Damage, float Impulse, AController* InstigatorController, AActor* DamageCauser)
{
	if (!ImpactActor)
	{
		return;
	}

	const float StrongImpulse = FMath::Max(Impulse, 3500.f);

	if (ABreakableTarget* Target = Cast<ABreakableTarget>(ImpactActor))
	{
		Target->BreakTarget(Direction * StrongImpulse, Hit.ImpactPoint, Hit.Item);
	}

	UGameplayStatics::ApplyPointDamage(
		ImpactActor,
		Damage,
		Direction,
		Hit,
		InstigatorController,
		DamageCauser,
		UDamageType::StaticClass());

	if (ImpactComponent && ImpactComponent->IsSimulatingPhysics())
	{
		ImpactComponent->WakeRigidBody(Hit.BoneName);
		ImpactComponent->AddImpulseAtLocation(Direction * StrongImpulse, Hit.ImpactPoint, Hit.BoneName);
	}

	if (!IsEnemyActor(ImpactActor))
	{
		return;
	}

	USkeletalMeshComponent* SkeletalMesh = Cast<USkeletalMeshComponent>(ImpactComponent);
	if (!SkeletalMesh)
	{
		SkeletalMesh = ImpactActor->FindComponentByClass<USkeletalMeshComponent>();
	}

	if (!SkeletalMesh || !SkeletalMesh->GetPhysicsAsset())
	{
		return;
	}

	const FName Bone = ResolveHitBone(SkeletalMesh, Hit);
	if (Bone.IsNone())
	{
		return;
	}

	const bool bHeadHit = IsHeadBone(Bone);

	// Keep a small independent health pool so the practice enemy reacts even
	// if its Blueprint does not implement AnyDamage.
	float& Health = EnemyHealth.FindOrAdd(ImpactActor, 100.f);
	Health -= Damage;

	if (bHeadHit || Health <= 0.f)
	{
		EnableRagdoll(SkeletalMesh);

		if (ACharacter* CharacterHit = Cast<ACharacter>(ImpactActor))
		{
			CharacterHit->GetCharacterMovement()->DisableMovement();
			if (AController* Controller = CharacterHit->GetController())
			{
				Controller->StopMovement();
			}
		}

		if (bHeadHit)
		{
			// Use the breakable mannequin Physics Asset supplied by the practice
			// project when available. This gives the neck a real breakable joint.
			if (UPhysicsAsset* BreakableAsset = LoadObject<UPhysicsAsset>(
				nullptr,
				TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin_PhysicsAsset_Breakable.SK_Mannequin_PhysicsAsset_Breakable")))
			{
				SkeletalMesh->SetPhysicsAsset(BreakableAsset, true);
			}

			SkeletalMesh->BreakConstraint(Direction * StrongImpulse, Hit.ImpactPoint, Bone);
			SkeletalMesh->SetBodySimulatePhysics(Bone, true);
			SkeletalMesh->WakeAllRigidBodies();
			SkeletalMesh->AddImpulseAtLocation(Direction * StrongImpulse * 3.5f, Hit.ImpactPoint, Bone);
		}
		else
		{
			SkeletalMesh->AddImpulseAtLocation(Direction * StrongImpulse, Hit.ImpactPoint, Bone);
		}

		EnemyHealth.Remove(ImpactActor);
		return;
	}

	PreparePhysicalAnimation(ImpactActor, SkeletalMesh, Bone);
	SkeletalMesh->SetBodySimulatePhysics(Bone, true);
	SkeletalMesh->SetAllBodiesBelowSimulatePhysics(Bone, true, true);
	SkeletalMesh->WakeAllRigidBodies();
	SkeletalMesh->AddImpulseAtLocation(Direction * StrongImpulse, Hit.ImpactPoint, Bone);
}

bool UPhysicsWeaponComponent::AttachWeapon(APhysicsCharacter* TargetCharacter)
{
	Character = TargetCharacter;

	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UPhysicsWeaponComponent>())
	{
		return false;
	}

	SetSimulatePhysics(false);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UPhysicsWeaponComponent::Fire);
		}
	}

	return true;
}

void UPhysicsWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character != nullptr)
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(FireMappingContext);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}
