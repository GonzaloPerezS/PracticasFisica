// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BreakableTarget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetBroken, ABreakableTarget*, target);

UCLASS()
class PHYSICS_API ABreakableTarget : public AActor
{
	GENERATED_BODY()

public:
	ABreakableTarget();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UGeometryCollectionComponent* GeometryCollection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	bool m_IsBroken = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Destruction)
	float m_DestroyDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Destruction)
	float m_BreakStrain = 100000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Destruction)
	float m_BreakRadius = 25.f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void GeometryCollectionBroken(const struct FChaosBreakEvent& BreakEvent);

	void DestroyAfterBreak();

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = Destruction)
	void BreakTarget(const FVector& Impulse, const FVector& Location, int32 HitItem = -1);

public:
	static FTargetBroken OnTargetBroken;
};
