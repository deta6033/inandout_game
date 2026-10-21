#include "InteractableDoor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AInteractableDoor::AInteractableDoor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
    DoorPivot->SetupAttachment(SceneRoot);

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(DoorPivot);
    DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void AInteractableDoor::BeginPlay()
{
    Super::BeginPlay();

    ClosedLocation = DoorPivot->GetRelativeLocation();
    ClosedRotation = DoorPivot->GetRelativeRotation();
    bOpen = bStartsOpen;

    if (bOpen)
    {
        if (DoorType == EInAndOutDoorType::Hinged)
        {
            FRotator StartRotation = ClosedRotation;
            StartRotation.Yaw += OpenAngle;
            DoorPivot->SetRelativeRotation(StartRotation);
        }
        else
        {
            DoorPivot->SetRelativeLocation(ClosedLocation + SlideOffset);
        }
    }
}

void AInteractableDoor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (DoorType == EInAndOutDoorType::Hinged)
    {
        FRotator TargetRotation = ClosedRotation;
        if (bOpen)
        {
            TargetRotation.Yaw += OpenAngle;
        }

        DoorPivot->SetRelativeRotation(
            FMath::RInterpTo(
                DoorPivot->GetRelativeRotation(),
                TargetRotation,
                DeltaSeconds,
                InterpSpeed));
    }
    else
    {
        const FVector TargetLocation = ClosedLocation + (bOpen ? SlideOffset : FVector::ZeroVector);
        DoorPivot->SetRelativeLocation(
            FMath::VInterpTo(
                DoorPivot->GetRelativeLocation(),
                TargetLocation,
                DeltaSeconds,
                InterpSpeed));
    }
}

void AInteractableDoor::Interact_Implementation(AActor* Interactor)
{
    if (bLocked)
    {
        return;
    }

    bOpen = !bOpen;
    BP_OnDoorStateChanged(bOpen);
}

FText AInteractableDoor::GetInteractionText_Implementation() const
{
    if (bLocked)
    {
        return FText::FromString(TEXT("잠겨 있음"));
    }

    return bOpen
        ? FText::FromString(TEXT("문 닫기"))
        : FText::FromString(TEXT("문 열기"));
}
