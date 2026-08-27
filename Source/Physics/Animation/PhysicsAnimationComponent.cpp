// Fill out your copyright notice in the Description page of Project Settings.


#include "PhysicsAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/ConstraintInstance.h"
#include <GameFramework/CharacterMovementComponent.h>
#include <GameFramework/Character.h>
#include "AIController.h"

// Sets default values for this component's properties
UPhysicsAnimationComponent::UPhysicsAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UPhysicsAnimationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* OwnerActor = GetOwner())
	{
		m_Mesh = GetOwner()->GetComponentByClass<USkeletalMeshComponent>();
	}
	
}

// Called every frame
void UPhysicsAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

