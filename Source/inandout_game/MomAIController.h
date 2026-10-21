#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MomAIController.generated.h"

class ACharacter;
class AMomCharacter;

enum class EMomAIState : uint8
{
    Patrol,
    Chase,
    Search
};

UCLASS()
class INANDOUT_GAME_API AMomAIController : public AAIController
{
    GENERATED_BODY()

public:
    AMomAIController();

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

private:
    UPROPERTY()
    TObjectPtr<AMomCharacter> MomCharacter;

    UPROPERTY(EditAnywhere, Category="AI", meta=(ClampMin="0.05"))
    float DecisionInterval = 0.20f;

    UPROPERTY(EditAnywhere, Category="AI|Sight", meta=(ClampMin="100.0"))
    float SightDistance = 1400.0f;

    UPROPERTY(EditAnywhere, Category="AI|Sight", meta=(ClampMin="1.0", ClampMax="179.0"))
    float SightAngleDegrees = 100.0f;

    UPROPERTY(EditAnywhere, Category="AI|Sight", meta=(ClampMin="0.1"))
    float LostSightSearchTime = 2.5f;

    UPROPERTY(EditAnywhere, Category="AI|Patrol", meta=(ClampMin="100.0"))
    float PatrolRadius = 900.0f;

    UPROPERTY(EditAnywhere, Category="AI|Patrol", meta=(ClampMin="0.0"))
    float PatrolPauseTime = 1.0f;

    UPROPERTY(EditAnywhere, Category="AI|Patrol", meta=(ClampMin="1.0"))
    float PatrolAcceptanceRadius = 60.0f;

    EMomAIState CurrentState = EMomAIState::Patrol;

    FVector LastSeenLocation = FVector::ZeroVector;
    float SearchStartTime = 0.0f;
    float NextPatrolMoveTime = 0.0f;
    bool bHasPatrolDestination = false;

    FTimerHandle DecisionTimerHandle;

    void MakeDecision();
    bool CanSeeSon(const ACharacter* Son) const;

    void UpdatePatrol(ACharacter* Son);
    void UpdateChase(ACharacter* Son);
    void UpdateSearch(ACharacter* Son);

    void EnterPatrol();
    void EnterChase(ACharacter* Son);
    void EnterSearch();
    void TryStartPatrolMove();

    UFUNCTION()
    void HandleMomDamaged(float ActualDamage, AActor* DamageCauser, AController* InstigatorController);
};
