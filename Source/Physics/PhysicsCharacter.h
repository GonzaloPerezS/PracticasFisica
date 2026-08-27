// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PhysicsCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UPhysicsHandleComponent;
class UPrimitiveComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FZoomInSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FZoomOutSignature);

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class APhysicsCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** PickUp Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PickUpAction;

	/** Camera FOV Zoom Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ZoomAction;

	/** Sprinting speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float m_SprintSpeedMultiplier;

	/** Maximum stamina, used when sprinting */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float m_MaxStamina;

	/** Stamina depletion rate, in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float m_StaminaDepletionRate;

	/** Stamina recovery rate, in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float m_StaminaRecoveryRate;

	/** Max health points */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float m_MaxHealth;

	/** Highlight overlay material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* m_HighlightMaterial;

	/** Maximum distance to allow component grabbing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float m_MaxGrabDistance = 1000.f;

	/** Base interpolation factor to move grabbed components */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float m_BaseInterpolationSpeed = 1000.f;

	/** Impulse applied when releasing a grabbed object. Set to 0 to simply drop it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float m_ThrowImpulse = 1600.f;

	UPROPERTY(BlueprintAssignable)
	FZoomInSignature OnZoomIn;

	UPROPERTY(BlueprintAssignable)
	FZoomOutSignature OnZoomOut;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DebugData, meta = (AllowPrivateAccess = "true"))
	float m_Sensitivity = 1.f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DebugData, meta = (AllowPrivateAccess = "true"))
	bool m_IsSprinting = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DebugData, meta = (AllowPrivateAccess = "true"))
	float m_CurrentStamina;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DebugData, meta = (AllowPrivateAccess = "true"))
	float m_CurrentHealth;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DebugData, meta = (AllowPrivateAccess = "true"))
	UPhysicsHandleComponent* m_PhysicsHandle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	UPrimitiveComponent* m_HighlightedComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	UPrimitiveComponent* m_GrabbedComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	float m_GrabDistance = 0.f;

public:
	APhysicsCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	void SetIsSprinting(bool NewIsSprinting);

protected:
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Sprint(const FInputActionValue& Value);
	void GrabObject(const FInputActionValue& Value);
	void ReleaseObject(const FInputActionValue& Value);
	void ZoomIn();
	void ZoomOut();
};
