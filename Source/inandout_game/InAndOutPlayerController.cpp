#include "InAndOutPlayerController.h"

#include "InAndOutHUD.h"
#include "InAndOutNativeWidgets.h"

#include "Blueprint/UserWidget.h"
#include "Components/InputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "InputCoreTypes.h"

AInAndOutPlayerController::AInAndOutPlayerController()
{
    PauseWidgetClass = UInAndOutPauseWidget::StaticClass();
    ResultWidgetClass = UInAndOutResultWidget::StaticClass();
}

void AInAndOutPlayerController::BeginPlay()
{
    Super::BeginPlay();

    ApplyGameInputMode();

    FTimerHandle ObjectiveTimer;
    GetWorldTimerManager().SetTimer(
        ObjectiveTimer,
        this,
        &AInAndOutPlayerController::ShowInitialObjective,
        0.15f,
        false);
}

void AInAndOutPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!InputComponent)
    {
        return;
    }

    FInputKeyBinding& PauseBinding = InputComponent->BindKey(
        EKeys::Escape,
        IE_Pressed,
        this,
        &AInAndOutPlayerController::TogglePauseMenu);

    PauseBinding.bExecuteWhenPaused = true;
}

void AInAndOutPlayerController::ShowInitialObjective()
{
    if (AInAndOutHUD* InAndOutHUD = Cast<AInAndOutHUD>(GetHUD()))
    {
        InAndOutHUD->ShowInitialObjective(4.0f);
    }
}

void AInAndOutPlayerController::ApplyGameInputMode()
{
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;
    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);
}

void AInAndOutPlayerController::ApplyUIInputMode(UUserWidget* FocusWidget)
{
    FInputModeGameAndUI InputMode;
    if (FocusWidget)
    {
        InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
    }

    SetInputMode(InputMode);
    bShowMouseCursor = true;
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
}

void AInAndOutPlayerController::TogglePauseMenu()
{
    if (bResultShown)
    {
        return;
    }

    if (PauseWidget)
    {
        ResumeBattle();
        return;
    }

    if (!PauseWidgetClass)
    {
        return;
    }

    PauseWidget = CreateWidget<UInAndOutPauseWidget>(this, PauseWidgetClass);
    if (!PauseWidget)
    {
        return;
    }

    SetPause(true);
    PauseWidget->AddToViewport(100);
    ApplyUIInputMode(PauseWidget);
}

void AInAndOutPlayerController::ResumeBattle()
{
    if (bResultShown)
    {
        return;
    }

    if (PauseWidget)
    {
        PauseWidget->RemoveFromParent();
        PauseWidget = nullptr;
    }

    SetPause(false);
    ApplyGameInputMode();
}

void AInAndOutPlayerController::ShowGameResult(bool bVictory, const FText& Message)
{
    if (bResultShown || !ResultWidgetClass)
    {
        return;
    }

    if (PauseWidget)
    {
        PauseWidget->RemoveFromParent();
        PauseWidget = nullptr;
    }

    ResultWidget = CreateWidget<UInAndOutResultWidget>(this, ResultWidgetClass);
    if (!ResultWidget)
    {
        return;
    }

    bResultShown = true;
    ResultWidget->Configure(bVictory, Message);
    ResultWidget->AddToViewport(200);

    SetPause(true);
    ApplyUIInputMode(ResultWidget);
}

void AInAndOutPlayerController::RestartCurrentBattle()
{
    SetPause(false);

    const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}

void AInAndOutPlayerController::ReturnToMainMenu()
{
    SetPause(false);
    UGameplayStatics::OpenLevel(this, MainMenuLevelName);
}

void AInAndOutPlayerController::NotifyHitMarker()
{
    if (AInAndOutHUD* InAndOutHUD = Cast<AInAndOutHUD>(GetHUD()))
    {
        InAndOutHUD->NotifyHitMarker();
    }
}

void AInAndOutPlayerController::NotifyPlayerDamaged()
{
    if (AInAndOutHUD* InAndOutHUD = Cast<AInAndOutHUD>(GetHUD()))
    {
        InAndOutHUD->NotifyPlayerDamaged();
    }
}

void AInAndOutPlayerController::NotifyEggSplat()
{
    if (AInAndOutHUD* InAndOutHUD = Cast<AInAndOutHUD>(GetHUD()))
    {
        InAndOutHUD->NotifyEggSplat();
    }
}
