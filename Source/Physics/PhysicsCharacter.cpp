// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsCharacter.h"
#include "Components/MeshComponent.h"
#include "PhysicsProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

APhysicsCharacter::APhysicsCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	m_PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}

void APhysicsCharacter::BeginPlay()
{
	Super::BeginPlay();
	m_CurrentStamina = m_MaxStamina;
	m_CurrentHealth = m_MaxHealth;
}

void APhysicsCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (FirstPersonCameraComponent)
	{
		const FVector Start = FirstPersonCameraComponent->GetComponentLocation();
		const FVector Forward = FirstPersonCameraComponent->GetForwardVector();
		const FVector End = Start + Forward * m_MaxGrabDistance;

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(PhysicsInteraction), true, this);
		Params.bReturnPhysicalMaterial = false;

		UPrimitiveComponent* NewHighlight = nullptr;
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			if (Hit.GetComponent() && Hit.GetComponent()->IsSimulatingPhysics())
			{
				NewHighlight = Hit.GetComponent();
			}
		}

		if (m_HighlightedComponent != NewHighlight)
		{
			if (m_HighlightedComponent)
			{
				m_HighlightedComponent->SetRenderCustomDepth(false);
				if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(m_HighlightedComponent))
				{
					MeshComponent->SetOverlayMaterial(nullptr);
				}
			}

			m_HighlightedComponent = NewHighlight;

			if (m_HighlightedComponent)
			{
				m_HighlightedComponent->SetRenderCustomDepth(true);
				if (m_HighlightMaterial)
				{
					if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(m_HighlightedComponent))
					{
						MeshComponent->SetOverlayMaterial(m_HighlightMaterial);
					}
				}
			}
		}

		if (m_PhysicsHandle && m_GrabbedComponent)
		{
			const FVector Target = Start + Forward * m_GrabDistance;
			m_PhysicsHandle->SetTargetLocationAndRotation(Target, FRotator::ZeroRotator);
		}
	}
}

void APhysicsCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APhysicsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Look);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APhysicsCharacter::Sprint);
		EnhancedInputComponent->BindAction(PickUpAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::GrabObject);
		EnhancedInputComponent->BindAction(PickUpAction, ETriggerEvent::Completed, this, &APhysicsCharacter::ReleaseObject);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &APhysicsCharacter::ZoomIn);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Completed, this, &APhysicsCharacter::ZoomOut);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}

void APhysicsCharacter::SetIsSprinting(bool NewIsSprinting)
{
	m_IsSprinting = NewIsSprinting && m_CurrentStamina > 0.f;
	const float BaseSpeed = 600.f;
	GetCharacterMovement()->MaxWalkSpeed = m_IsSprinting ? BaseSpeed * m_SprintSpeedMultiplier : BaseSpeed;
}

void APhysicsCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APhysicsCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X * m_Sensitivity);
		AddControllerPitchInput(LookAxisVector.Y * m_Sensitivity);
	}
}

void APhysicsCharacter::Sprint(const FInputActionValue& Value)
{
	SetIsSprinting(Value.Get<bool>());
}

void APhysicsCharacter::GrabObject(const FInputActionValue& Value)
{
	if (!FirstPersonCameraComponent || !m_PhysicsHandle || m_GrabbedComponent)
	{
		return;
	}

	const FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	const FVector Forward = FirstPersonCameraComponent->GetForwardVector();
	const FVector End = Start + Forward * m_MaxGrabDistance;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(GrabPhysicsObject), true, this);
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return;
	}

	UPrimitiveComponent* Component = Hit.GetComponent();
	if (!Component)
	{
		return;
	}

	if (!Component->IsSimulatingPhysics() && Hit.GetActor() && Hit.GetActor()->GetClass()->GetName().Contains(TEXT("Door")))
	{
		Component->SetSimulatePhysics(true);
	}

	if (!Component->IsSimulatingPhysics())
	{
		return;
	}

	m_GrabDistance = FMath::Clamp(FVector::Distance(Start, Hit.ImpactPoint), 100.f, m_MaxGrabDistance);
	m_PhysicsHandle->GrabComponentAtLocationWithRotation(Component, Hit.BoneName, Hit.ImpactPoint, Component->GetComponentRotation());
	m_GrabbedComponent = Component;
}

void APhysicsCharacter::ReleaseObject(const FInputActionValue& Value)
{
	if (!m_PhysicsHandle || !m_GrabbedComponent)
	{
		return;
	}

	UPrimitiveComponent* Component = m_GrabbedComponent;
	const FVector ThrowDirection = FirstPersonCameraComponent ? FirstPersonCameraComponent->GetForwardVector() : GetActorForwardVector();

	m_PhysicsHandle->ReleaseComponent();
	m_GrabbedComponent = nullptr;

	if (Component && m_ThrowImpulse > 0.f && Component->IsSimulatingPhysics())
	{
		Component->AddImpulse(ThrowDirection * m_ThrowImpulse, NAME_None, true);
	}

	m_GrabDistance = 0.f;
}

void APhysicsCharacter::ZoomIn()
{
	m_Sensitivity = 0.5f;
	OnZoomIn.Broadcast();
}

void APhysicsCharacter::ZoomOut()
{
	m_Sensitivity = 1.f;
	OnZoomOut.Broadcast();
}
