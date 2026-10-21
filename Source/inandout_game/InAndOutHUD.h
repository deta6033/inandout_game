#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "InAndOutHUD.generated.h"

class UFont;
class UTexture2D;

UCLASS()
class INANDOUT_GAME_API AInAndOutHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

    void ShowInitialObjective(float Duration = 4.0f);
    void NotifyHitMarker(float Duration = 0.16f);
    void NotifyPlayerDamaged(float Duration = 0.38f);
    void NotifyEggSplat(float Duration = 1.6f);

private:
    UPROPERTY(EditDefaultsOnly, Category="HUD")
    TObjectPtr<UFont> HUDFont;

    UPROPERTY(EditDefaultsOnly, Category="HUD")
    TObjectPtr<UTexture2D> EggSplatTexture;

    float ObjectiveEndTime = -1.0f;
    float HitMarkerEndTime = -1.0f;
    float DamageFlashEndTime = -1.0f;
    float DamageFlashDuration = 0.38f;
    float EggSplatEndTime = -1.0f;
    float EggSplatDuration = 1.6f;

    void DrawBar(float X, float Y, float Width, float Height, float NormalizedValue, const FLinearColor& FillColor);
    UFont* ResolveFont() const;
};
