#include "InAndOutGameMode.h"

#include "HealthComponent.h"
#include "InAndOutHUD.h"
#include "InAndOutPlayerController.h"
#include "MomCharacter.h"
#include "SonCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AInAndOutGameMode::AInAndOutGameMode()
{
    DefaultPawnClass = ASonCharacter::StaticClass();
    PlayerControllerClass = AInAndOutPlayerController::StaticClass();
    HUDClass = AInAndOutHUD::StaticClass();
}

void AInAndOutGameMode::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimer(
        BindTimerHandle,
        this,
        &AInAndOutGameMode::TryBindCombatants,
        0.25f,
        true,
        0.05f);
}

void AInAndOutGameMode::TryBindCombatants()
{
    if (!SonCharacter)
    {
        SonCharacter = Cast<ASonCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
        if (SonCharacter && SonCharacter->GetHealthComponent())
        {
            SonCharacter->GetHealthComponent()->OnDeath.AddUniqueDynamic(
                this,
                &AInAndOutGameMode::HandleSonDeath);
        }
    }

    if (!MomCharacter)
    {
        TArray<AActor*> FoundMoms;
        UGameplayStatics::GetAllActorsOfClass(this, AMomCharacter::StaticClass(), FoundMoms);

        if (FoundMoms.Num() > 0)
        {
            MomCharacter = Cast<AMomCharacter>(FoundMoms[0]);
            if (MomCharacter && MomCharacter->GetHealthComponent())
            {
                MomCharacter->GetHealthComponent()->OnDeath.AddUniqueDynamic(
                    this,
                    &AInAndOutGameMode::HandleMomDeath);
            }
        }
    }

    if (SonCharacter && MomCharacter)
    {
        GetWorldTimerManager().ClearTimer(BindTimerHandle);
    }
}

void AInAndOutGameMode::HandleSonDeath(AActor* DamageCauser)
{
    FinishMatch(false);
}

void AInAndOutGameMode::HandleMomDeath(AActor* DamageCauser)
{
    FinishMatch(true);
}

void AInAndOutGameMode::FinishMatch(bool bVictory)
{
    if (bMatchEnded)
    {
        return;
    }

    bMatchEnded = true;
    GetWorldTimerManager().ClearTimer(BindTimerHandle);

    const TArray<FText> VictoryMessages =
    {
        FText::FromString(TEXT("알았어. 오늘은 게임해.")),
        FText::FromString(TEXT("그래, 오늘은 게임해도 돼.")),
        FText::FromString(TEXT("이번에는 네가 이겼다. 게임해."))
    };

    const TArray<FText> DefeatMessages =
    {
        FText::FromString(TEXT("공부부터 해.")),
        FText::FromString(TEXT("게임기는 다시 압수야.")),
        FText::FromString(TEXT("방부터 치워.")),
        FText::FromString(TEXT("숙제 다 했어?")),
        FText::FromString(TEXT("집안일 좀 해."))
    };

    const TArray<FText>& Messages = bVictory ? VictoryMessages : DefeatMessages;
    const FText SelectedMessage = Messages[FMath::RandRange(0, Messages.Num() - 1)];

    if (AInAndOutPlayerController* PC = Cast<AInAndOutPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
    {
        PC->ShowGameResult(bVictory, SelectedMessage);
    }
}
