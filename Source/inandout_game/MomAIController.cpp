#include "MomAIController.h"

#include "HealthComponent.h"
#include "MomCharacter.h"

#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

AMomAIController::AMomAIController()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMomAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    MomCharacter = Cast<AMomCharacter>(InPawn);
    if (!MomCharacter)
    {
        return;
    }

    if (UHealthComponent* Health = MomCharacter->GetHealthComponent())
    {
        Health->OnDamaged.AddUniqueDynamic(this, &AMomAIController::HandleMomDamaged);
    }

    EnterPatrol();

    GetWorldTimerManager().SetTimer(
        DecisionTimerHandle,
        this,
        &AMomAIController::MakeDecision,
        DecisionInterval,
        true,
        0.1f);
}

void AMomAIController::OnUnPossess()
{
    GetWorldTimerManager().ClearTimer(DecisionTimerHandle);

    if (MomCharacter)
    {
        if (UHealthComponent* Health = MomCharacter->GetHealthComponent())
        {
            Health->OnDamaged.RemoveDynamic(this, &AMomAIController::HandleMomDamaged);
        }
    }

    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
    MomCharacter = nullptr;

    Super::OnUnPossess();
}

void AMomAIController::MakeDecision()
{
    if (!MomCharacter || MomCharacter->IsDead())
    {
        StopMovement();
        ClearFocus(EAIFocusPriority::Gameplay);
        return;
    }

    if (MomCharacter->IsStunned())
    {
        StopMovement();
        ClearFocus(EAIFocusPriority::Gameplay);
        return;
    }

    ACharacter* Son = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Son)
    {
        return;
    }

    if (MomCharacter->IsAttackLocked())
    {
        if (CurrentState == EMomAIState::Chase) UpdateAttackMovement(Son);
        return;
    }

    switch (CurrentState)
    {
    case EMomAIState::Patrol:
        UpdatePatrol(Son);
        break;

    case EMomAIState::Chase:
        UpdateChase(Son);
        break;

    case EMomAIState::Search:
        UpdateSearch(Son);
        break;

    default:
        EnterPatrol();
        break;
    }
}

bool AMomAIController::CanSeeSon(const ACharacter* Son) const
{
    if (!MomCharacter || !Son)
    {
        return false;
    }

    FVector ToSon = Son->GetActorLocation() - MomCharacter->GetActorLocation();
    const float Distance = ToSon.Size();

    if (Distance > SightDistance || Distance <= KINDA_SMALL_NUMBER)
    {
        return false;
    }

    // 실제 엄마 몸이 바라보는 방향을 기준으로 시야 판정
    const FVector SightForward =
        MomCharacter->GetActorForwardVector().GetSafeNormal2D();

    const FVector DirectionToSon = ToSon.GetSafeNormal2D();

    const float HalfAngleRadians =
        FMath::DegreesToRadians(SightAngleDegrees * 0.5f);

    const float MinimumDot =
        FMath::Cos(HalfAngleRadians);

    if (FVector::DotProduct(SightForward, DirectionToSon) < MinimumDot)
    {
        return false;
    }

    return LineOfSightTo(Son);
}

void AMomAIController::UpdatePatrol(ACharacter* Son)
{
    if (CanSeeSon(Son))
    {
        EnterChase(Son);
        return;
    }

    if (GetMoveStatus() == EPathFollowingStatus::Moving)
    {
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();

    if (bHasPatrolDestination)
    {
        bHasPatrolDestination = false;
        NextPatrolMoveTime = Now + PatrolPauseTime;
        return;
    }

    if (Now >= NextPatrolMoveTime)
    {
        TryStartPatrolMove();
    }
}

void AMomAIController::UpdateChase(ACharacter* Son)
{
    if (!CanSeeSon(Son))
    {
        EnterSearch();
        return;
    }

    LastSeenLocation = Son->GetActorLocation();
    SetFocus(Son, EAIFocusPriority::Gameplay);

    const float Distance = FVector::Dist(
        MomCharacter->GetActorLocation(),
        Son->GetActorLocation());

    if (MomCharacter->IsUltimateReady()
        && Distance <= MomCharacter->GetUltimateRange())
    {
        if (MomCharacter->BeginTelegraphedAttack(Son, 2))
        {
            return;
        }
    }

    if (Distance <= MomCharacter->GetMeleeRange())
    {
        if (MomCharacter->BeginTelegraphedAttack(Son, 0)) return;
        MoveToActor(Son, MomCharacter->GetMeleeRange() * 0.65f);
        return;
    }

    if (Distance <= MomCharacter->GetRangedRange())
    {
        if (MomCharacter->BeginTelegraphedAttack(Son, 1))
        {
            bStrafeClockwise = !bStrafeClockwise;
            return;
        }
        // 달걀 쿨타임 중이면 멈추지 않고 계속 접근한다.
    }

    MoveToActor(
        Son,
        MomCharacter->GetMeleeRange() * 0.80f,
        true,
        true,
        true);
}

void AMomAIController::UpdateAttackMovement(ACharacter* Son)
{
    if (!MomCharacter || !IsValid(Son)) return;
    SetFocus(Son, EAIFocusPriority::Gameplay);
    const int32 Attack = MomCharacter->GetActiveAttackIndex();
    if (Attack == 0) // Melee: pursue through the telegraph.
    {
        MoveToActor(Son, MomCharacter->GetMeleeRange() * 0.65f);
    }
    else if (Attack == 1) // Egg: lateral repositioning around the player.
    {
        const FVector Away = (MomCharacter->GetActorLocation() - Son->GetActorLocation()).GetSafeNormal2D();
        if (Away.IsNearlyZero()) return;
        const FVector Side = FVector::CrossProduct(FVector::UpVector, Away);
        const float Sign = bStrafeClockwise ? 1.0f : -1.0f;
        const FVector Desired = Son->GetActorLocation() + Away * EggPreferredDistance + Side * (Sign * EggStrafeOffset);
        MoveToLocation(Desired, 70.0f);
    }
    else // Ultimate and recovery: slow pressure, no hard stop.
    {
        MoveToActor(Son, MomCharacter->GetMeleeRange() * 0.85f);
    }
}

void AMomAIController::UpdateSearch(ACharacter* Son)
{
    if (CanSeeSon(Son))
    {
        EnterChase(Son);
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - SearchStartTime >= LostSightSearchTime)
    {
        EnterPatrol();
        return;
    }

    SetFocalPoint(LastSeenLocation, EAIFocusPriority::Gameplay);

    if (GetMoveStatus() != EPathFollowingStatus::Moving)
    {
        MoveToLocation(
            LastSeenLocation,
            PatrolAcceptanceRadius,
            true,
            true,
            true,
            false,
            nullptr,
            true);
    }
}

void AMomAIController::EnterPatrol()
{
    CurrentState = EMomAIState::Patrol;
    SearchStartTime = 0.0f;
    bHasPatrolDestination = false;
    NextPatrolMoveTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
}

void AMomAIController::EnterChase(ACharacter* Son)
{
    if (!Son)
    {
        return;
    }

    CurrentState = EMomAIState::Chase;
    LastSeenLocation = Son->GetActorLocation();
    SearchStartTime = 0.0f;
    bHasPatrolDestination = false;

    SetFocus(Son, EAIFocusPriority::Gameplay);
    MoveToActor(
        Son,
        MomCharacter ? MomCharacter->GetMeleeRange() * 0.80f : 50.0f,
        true,
        true,
        true);
}

void AMomAIController::EnterSearch()
{
    CurrentState = EMomAIState::Search;
    SearchStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    ClearFocus(EAIFocusPriority::Gameplay);
    SetFocalPoint(LastSeenLocation, EAIFocusPriority::Gameplay);

    MoveToLocation(
        LastSeenLocation,
        PatrolAcceptanceRadius,
        true,
        true,
        true,
        false,
        nullptr,
        true);
}

void AMomAIController::TryStartPatrolMove()
{
    if (!MomCharacter)
    {
        return;
    }

    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem)
    {
        return;
    }

    FNavLocation RandomLocation;
    if (!NavSystem->GetRandomReachablePointInRadius(
            MomCharacter->GetActorLocation(),
            PatrolRadius,
            RandomLocation))
    {
        NextPatrolMoveTime = GetWorld()->GetTimeSeconds() + PatrolPauseTime;
        return;
    }

    ClearFocus(EAIFocusPriority::Gameplay);
    MoveToLocation(
        RandomLocation.Location,
        PatrolAcceptanceRadius,
        true,
        true,
        true,
        false,
        nullptr,
        true);

    bHasPatrolDestination = true;
}

void AMomAIController::HandleMomDamaged(
    float ActualDamage,
    AActor* DamageCauser,
    AController* InstigatorController)
{
    if (ActualDamage <= 0.0f || !MomCharacter || MomCharacter->IsDead())
    {
        return;
    }

    ACharacter* Son = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!Son)
    {
        return;
    }

    const bool bDamagedBySon =
        DamageCauser == Son
        || (DamageCauser && DamageCauser->GetInstigator() == Son)
        || (InstigatorController && InstigatorController->GetPawn() == Son);

    if (bDamagedBySon)
    {
        EnterChase(Son);
    }
}
