#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InAndOutNativeWidgets.generated.h"

class UBorder;
class UTextBlock;

UCLASS()
class INANDOUT_GAME_API UInAndOutMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void HandlePlayClicked();

    UFUNCTION()
    void HandleExitClicked();
};

UCLASS()
class INANDOUT_GAME_API UInAndOutIntroWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Intro")
    void SetStoryLine(const FText& NewStoryLine);

protected:
    virtual void NativeOnInitialized() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> StoryText;

    FText PendingStoryLine;
};

UCLASS()
class INANDOUT_GAME_API UInAndOutResultWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void Configure(bool bInVictory, const FText& InMessage);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UBorder> FadeBackground;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ResultTitleText;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ResultMessageText;

    bool bVictory = false;
    FText ResultMessage;
    float FadeAlpha = 0.0f;

    void ApplyResultText();

    UFUNCTION()
    void HandleRetryClicked();

    UFUNCTION()
    void HandleMainMenuClicked();
};

UCLASS()
class INANDOUT_GAME_API UInAndOutPauseWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void HandleResumeClicked();

    UFUNCTION()
    void HandleRetryClicked();

    UFUNCTION()
    void HandleMainMenuClicked();
};
