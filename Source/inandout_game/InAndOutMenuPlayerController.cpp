#include "InAndOutMenuPlayerController.h"

#include "InAndOutNativeWidgets.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

AInAndOutMenuPlayerController::AInAndOutMenuPlayerController()
{
    MainMenuWidgetClass = UInAndOutMainMenuWidget::StaticClass();
    IntroWidgetClass = UInAndOutIntroWidget::StaticClass();
}

void AInAndOutMenuPlayerController::BeginPlay()
{
    Super::BeginPlay();

    StoryLines =
    {
        FText::FromString(TEXT("엄마가 내 게임기를 압수했다.")),
        FText::FromString(TEXT("그래서 몰래 되찾았다.")),
        FText::FromString(TEXT("그런데...")),
        FText::FromString(TEXT("들켰다."))
    };

    ShowMainMenu();
}

void AInAndOutMenuPlayerController::ShowMainMenu()
{
    if (!MainMenuWidgetClass)
    {
        return;
    }

    MainMenuWidget = CreateWidget<UInAndOutMainMenuWidget>(this, MainMenuWidgetClass);
    if (!MainMenuWidget)
    {
        return;
    }

    MainMenuWidget->AddToViewport(10);

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
    SetInputMode(InputMode);
    bShowMouseCursor = true;
}

void AInAndOutMenuPlayerController::StartGameFromMenu()
{
    if (MainMenuWidget)
    {
        MainMenuWidget->RemoveFromParent();
        MainMenuWidget = nullptr;
    }

    if (!IntroWidgetClass)
    {
        UGameplayStatics::OpenLevel(this, BattleLevelName);
        return;
    }

    IntroWidget = CreateWidget<UInAndOutIntroWidget>(this, IntroWidgetClass);
    if (!IntroWidget)
    {
        UGameplayStatics::OpenLevel(this, BattleLevelName);
        return;
    }

    IntroWidget->AddToViewport(20);
    CurrentStoryLine = 0;
    AdvanceIntro();
}

void AInAndOutMenuPlayerController::AdvanceIntro()
{
    if (CurrentStoryLine >= StoryLines.Num())
    {
        GetWorldTimerManager().ClearTimer(IntroTimerHandle);
        UGameplayStatics::OpenLevel(this, BattleLevelName);
        return;
    }

    if (IntroWidget)
    {
        IntroWidget->SetStoryLine(StoryLines[CurrentStoryLine]);
    }

    ++CurrentStoryLine;

    GetWorldTimerManager().SetTimer(
        IntroTimerHandle,
        this,
        &AInAndOutMenuPlayerController::AdvanceIntro,
        StoryLineDuration,
        false);
}

void AInAndOutMenuPlayerController::QuitGameFromMenu()
{
    UKismetSystemLibrary::QuitGame(
        this,
        this,
        EQuitPreference::Quit,
        true);
}
