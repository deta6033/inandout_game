#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InAndOutInteractable.h"
#include "InteractableDoor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EInAndOutDoorType : uint8
{
    Hinged  UMETA(DisplayName="Hinged / Rotating"),
    Sliding UMETA(DisplayName="Sliding")
};

UCLASS()
class INANDOUT_GAME_API AInteractableDoor : public AActor, public IInAndOutInteractable
{
    GENERATED_BODY()

public:
    AInteractableDoor();

    virtual void Tick(float DeltaSeconds) override;

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual FText GetInteractionText_Implementation() const override;

    UFUNCTION(BlueprintPure, Category="Door")
    bool IsOpen() const { return bOpen; }

protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Category="Door", meta=(DisplayName="On Door State Changed"))
    void BP_OnDoorStateChanged(bool bNowOpen);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USceneComponent> DoorPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UStaticMeshComponent> DoorMesh;

    UPROPERTY(EditAnywhere, Category="Door")
    EInAndOutDoorType DoorType = EInAndOutDoorType::Hinged;

    UPROPERTY(EditAnywhere, Category="Door")
    bool bStartsOpen = false;

    UPROPERTY(EditAnywhere, Category="Door")
    bool bLocked = false;

    UPROPERTY(EditAnywhere, Category="Door", meta=(ClampMin="0.1"))
    float InterpSpeed = 5.0f;

    UPROPERTY(EditAnywhere, Category="Door|Hinged")
    float OpenAngle = 95.0f;

    UPROPERTY(EditAnywhere, Category="Door|Sliding")
    FVector SlideOffset = FVector(120.0f, 0.0f, 0.0f);

    bool bOpen = false;
    FVector ClosedLocation = FVector::ZeroVector;
    FRotator ClosedRotation = FRotator::ZeroRotator;
};
