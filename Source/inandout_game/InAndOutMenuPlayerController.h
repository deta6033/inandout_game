#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InAndOutMenuPlayerController.generated.h"

class UInAndOutIntroWidget;
class UInAndOutMainMenuWidget;

UCLASS()
class INANDOUT_GAME_API AInAndOutMenuPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AInAndOutMenuPlayerController();

    UFUNCTION(BlueprintCallable, Category="Menu")
    void StartGameFromMenu();

    UFUNCTION(BlueprintCallable, Category="Menu")
    void QuitGameFromMenu();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UInAndOutMainMenuWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UInAndOutIntroWidget> IntroWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category="Game Flow")
    FName BattleLevelName = TEXT("Battle_Home");

    UPROPERTY(EditDefaultsOnly, Category="Game Flow", meta=(ClampMin="0.2"))
    float StoryLineDuration = 1.25f;

    UPROPERTY(Transient)
    TObjectPtr<UInAndOutMainMenuWidget> MainMenuWidget;

    UPROPERTY(Transient)
    TObjectPtr<UInAndOutIntroWidget> IntroWidget;

    TArray<FText> StoryLines;
    int32 CurrentStoryLine = 0;
    FTimerHandle IntroTimerHandle;

    void ShowMainMenu();
    void AdvanceIntro();
};
