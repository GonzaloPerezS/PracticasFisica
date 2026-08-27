#include "BreakableTarget.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "TimerManager.h"

FTargetBroken ABreakableTarget::OnTargetBroken;

ABreakableTarget::ABreakableTarget()
{
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	StaticMesh->SetCollisionResponseToAllChannels(ECR_Block);

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	GeometryCollection->SetupAttachment(StaticMesh);
	GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableTarget::GeometryCollectionBroken);
	GeometryCollection->SetNotifyBreaks(true);
	GeometryCollection->bEnableDamageFromCollision = true;
	GeometryCollection->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GeometryCollection->SetSimulatePhysics(false);
	GeometryCollection->SetVisibility(false);
}

void ABreakableTarget::BeginPlay()
{
	Super::BeginPlay();
}

void ABreakableTarget::GeometryCollectionBroken(const FChaosBreakEvent& BreakEvent)
{
	if (m_IsBroken)
	{
		return;
	}

	m_IsBroken = true;
	OnTargetBroken.Broadcast(this);

	StaticMesh->SetVisibility(false);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GeometryCollection->SetVisibility(true);
	GeometryCollection->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GeometryCollection->SetSimulatePhysics(true);
	GeometryCollection->WakeAllRigidBodies();

	FTimerHandle DestroyTimer;
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&ABreakableTarget::DestroyAfterBreak,
		FMath::Max(0.f, m_DestroyDelay),
		false);
}

void ABreakableTarget::DestroyAfterBreak()
{
	if (IsValid(this))
	{
		Destroy();
	}
}

void ABreakableTarget::BreakTarget(const FVector& Impulse, const FVector& Location, int32 HitItem)
{
	if (m_IsBroken || !GeometryCollection)
	{
		return;
	}

	GeometryCollection->SetVisibility(true);
	GeometryCollection->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GeometryCollection->SetSimulatePhysics(true);
	GeometryCollection->bEnableDamageFromCollision = true;
	GeometryCollection->SetNotifyBreaks(true);
	GeometryCollection->WakeAllRigidBodies();

	// Hit.Item is the Geometry Collection piece index. Use it directly when
	// available; unlike a hard-coded zero it identifies the actual piece hit.
	const int32 ItemIndex = HitItem >= 0 ? HitItem : 0;
	const float Strain = FMath::Max(m_BreakStrain, Impulse.Size() * 20.f);
	const float Radius = FMath::Max(50.f, m_BreakRadius);

	GeometryCollection->ApplyExternalStrain(
		ItemIndex,
		Location,
		Radius,
		4,
		0.75f,
		Strain);

	GeometryCollection->ApplyInternalStrain(
		ItemIndex,
		Location,
		Radius,
		2,
		0.5f,
		Strain * 0.5f);

	GeometryCollection->AddImpulseAtLocation(Impulse, Location, NAME_None);
	GeometryCollection->AddRadialImpulse(
		Location,
		Radius * 2.f,
		FMath::Max(Impulse.Size() * 0.5f, 2500.f),
		ERadialImpulseFalloff::RIF_Linear,
		true);
}

void ABreakableTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
