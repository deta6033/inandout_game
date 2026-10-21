#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InAndOutPlayerController.generated.h"

class UUserWidget;
class UInAndOutPauseWidget;
class UInAndOutResultWidget;

UCLASS()
class INANDOUT_GAME_API AInAndOutPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AInAndOutPlayerController();

    virtual void SetupInputComponent() override;

    void ShowGameResult(bool bVictory, const FText& Message);
    void NotifyHitMarker();
    void NotifyPlayerDamaged();
    void NotifyEggSplat();

    UFUNCTION(BlueprintCallable, Category="Game Flow")
    void ResumeBattle();

    UFUNCTION(BlueprintCallable, Category="Game Flow")
    void RestartCurrentBattle();

    UFUNCTION(BlueprintCallable, Category="Game Flow")
    void ReturnToMainMenu();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UInAndOutPauseWidget> PauseWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UInAndOutResultWidget> ResultWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category="Game Flow")
    FName MainMenuLevelName = TEXT("L_MainMenu");

    UPROPERTY(Transient)
    TObjectPtr<UInAndOutPauseWidget> PauseWidget;

    UPROPERTY(Transient)
    TObjectPtr<UInAndOutResultWidget> ResultWidget;

    bool bResultShown = false;

    void TogglePauseMenu();
    void ShowInitialObjective();
    void ApplyGameInputMode();
    void ApplyUIInputMode(UUserWidget* FocusWidget);
};
