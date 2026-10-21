#include "InAndOutHUD.h"

#include "HealthComponent.h"
#include "MomCharacter.h"
#include "SonCharacter.h"
#include "UltimateChargeComponent.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

void AInAndOutHUD::ShowInitialObjective(float Duration)
{
    ObjectiveEndTime = GetWorld()->GetTimeSeconds() + FMath::Max(0.1f, Duration);
}

void AInAndOutHUD::NotifyHitMarker(float Duration)
{
    HitMarkerEndTime = GetWorld()->GetTimeSeconds() + FMath::Max(0.05f, Duration);
}

void AInAndOutHUD::NotifyPlayerDamaged(float Duration)
{
    DamageFlashDuration = FMath::Max(0.05f, Duration);
    DamageFlashEndTime = GetWorld()->GetTimeSeconds() + DamageFlashDuration;
}

void AInAndOutHUD::NotifyEggSplat(float Duration)
{
    EggSplatDuration = FMath::Max(0.05f, Duration);
    EggSplatEndTime = GetWorld()->GetTimeSeconds() + EggSplatDuration;
}

UFont* AInAndOutHUD::ResolveFont() const
{
    if (HUDFont)
    {
        return HUDFont;
    }

    return GEngine ? GEngine->GetSmallFont() : nullptr;
}

void AInAndOutHUD::DrawBar(
    float X,
    float Y,
    float Width,
    float Height,
    float NormalizedValue,
    const FLinearColor& FillColor)
{
    const float ClampedValue = FMath::Clamp(NormalizedValue, 0.0f, 1.0f);
    DrawRect(FLinearColor(0.02f, 0.02f, 0.02f, 0.82f), X, Y, Width, Height);
    DrawRect(FillColor, X + 2.0f, Y + 2.0f, (Width - 4.0f) * ClampedValue, Height - 4.0f);
}

void AInAndOutHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas || !PlayerOwner)
    {
        return;
    }

    const float ScreenWidth = Canvas->ClipX;
    const float ScreenHeight = Canvas->ClipY;
    const float Now = GetWorld()->GetTimeSeconds();
    UFont* Font = ResolveFont();

    ASonCharacter* Son = Cast<ASonCharacter>(PlayerOwner->GetPawn());
    AMomCharacter* Mom = nullptr;

    for (TActorIterator<AMomCharacter> It(GetWorld()); It; ++It)
    {
        Mom = *It;
        break;
    }

    if (Son)
    {
        const UHealthComponent* SonHealth = Son->GetHealthComponent();
        const UUltimateChargeComponent* SonUltimate = Son->GetUltimateComponent();

        DrawText(TEXT("아들 HP"), FLinearColor::White, 35.0f, ScreenHeight - 120.0f, Font, 1.0f, false);
        DrawBar(35.0f, ScreenHeight - 95.0f, 280.0f, 22.0f, SonHealth->GetHealthNormalized(), FLinearColor(0.12f, 0.65f, 1.0f, 1.0f));

        DrawText(TEXT("성적표 궁극기"), FLinearColor::White, 35.0f, ScreenHeight - 62.0f, Font, 0.9f, false);
        DrawBar(35.0f, ScreenHeight - 40.0f, 280.0f, 16.0f, SonUltimate->GetChargeNormalized(), FLinearColor(0.72f, 0.28f, 1.0f, 1.0f));

        const FString AmmoText = Son->IsReloading()
            ? TEXT("재장전 중...")
            : FString::Printf(TEXT("폼다트 %d / %d  (예비탄 무한)"), Son->GetCurrentMagazineAmmo(), Son->GetMagazineSize());

        DrawText(
            AmmoText,
            Son->IsReloading() ? FLinearColor(1.0f, 0.82f, 0.2f, 1.0f) : FLinearColor::White,
            ScreenWidth - 330.0f,
            ScreenHeight - 60.0f,
            Font,
            1.0f,
            false);

        const FText Prompt = Son->GetInteractionPrompt();
        if (!Prompt.IsEmpty())
        {
            DrawText(
                FString::Printf(TEXT("[E] %s"), *Prompt.ToString()),
                FLinearColor::White,
                ScreenWidth * 0.5f - 70.0f,
                ScreenHeight * 0.62f,
                Font,
                1.05f,
                false);
        }
    }

    if (Mom)
    {
        const UHealthComponent* MomHealth = Mom->GetHealthComponent();
        const UUltimateChargeComponent* MomUltimate = Mom->GetUltimateComponent();

        DrawText(TEXT("엄마"), FLinearColor::White, ScreenWidth * 0.5f - 24.0f, 28.0f, Font, 1.1f, false);
        DrawBar(ScreenWidth * 0.5f - 250.0f, 52.0f, 500.0f, 24.0f, MomHealth->GetHealthNormalized(), FLinearColor(1.0f, 0.22f, 0.34f, 1.0f));
        DrawBar(ScreenWidth * 0.5f - 250.0f, 82.0f, 500.0f, 11.0f, MomUltimate->GetChargeNormalized(), FLinearColor(1.0f, 0.70f, 0.1f, 1.0f));
    }

    const float CenterX = ScreenWidth * 0.5f;
    const float CenterY = ScreenHeight * 0.5f;

    DrawLine(CenterX - 7.0f, CenterY, CenterX + 7.0f, CenterY, FLinearColor(1.0f, 1.0f, 1.0f, 0.75f), 1.5f);
    DrawLine(CenterX, CenterY - 7.0f, CenterX, CenterY + 7.0f, FLinearColor(1.0f, 1.0f, 1.0f, 0.75f), 1.5f);

    if (Now < HitMarkerEndTime)
    {
        const FLinearColor HitColor(1.0f, 1.0f, 1.0f, 1.0f);
        DrawLine(CenterX - 13.0f, CenterY - 13.0f, CenterX - 4.0f, CenterY - 4.0f, HitColor, 2.5f);
        DrawLine(CenterX + 13.0f, CenterY - 13.0f, CenterX + 4.0f, CenterY - 4.0f, HitColor, 2.5f);
        DrawLine(CenterX - 13.0f, CenterY + 13.0f, CenterX - 4.0f, CenterY + 4.0f, HitColor, 2.5f);
        DrawLine(CenterX + 13.0f, CenterY + 13.0f, CenterX + 4.0f, CenterY + 4.0f, HitColor, 2.5f);
    }

    if (Now < ObjectiveEndTime)
    {
        DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.58f), ScreenWidth * 0.5f - 255.0f, ScreenHeight * 0.20f, 510.0f, 105.0f);
        DrawText(TEXT("목표: 게임기를 지켜라!"), FLinearColor(1.0f, 0.88f, 0.2f, 1.0f), ScreenWidth * 0.5f - 135.0f, ScreenHeight * 0.20f + 20.0f, Font, 1.25f, false);
        DrawText(TEXT("엄마를 쓰러뜨리고 게임할 권리를 되찾아라."), FLinearColor::White, ScreenWidth * 0.5f - 205.0f, ScreenHeight * 0.20f + 60.0f, Font, 0.95f, false);
    }

    if (Now < DamageFlashEndTime)
    {
        const float Remaining = DamageFlashEndTime - Now;
        const float Alpha = FMath::Clamp(Remaining / DamageFlashDuration, 0.0f, 1.0f) * 0.40f;
        const FLinearColor FlashColor(1.0f, 0.02f, 0.02f, Alpha);
        const float Thickness = 55.0f;

        DrawRect(FlashColor, 0.0f, 0.0f, ScreenWidth, Thickness);
        DrawRect(FlashColor, 0.0f, ScreenHeight - Thickness, ScreenWidth, Thickness);
        DrawRect(FlashColor, 0.0f, 0.0f, Thickness, ScreenHeight);
        DrawRect(FlashColor, ScreenWidth - Thickness, 0.0f, Thickness, ScreenHeight);
    }

    if (Now < EggSplatEndTime)
    {
        const float Remaining = EggSplatEndTime - Now;
        const float Alpha = FMath::Clamp(Remaining / EggSplatDuration, 0.0f, 1.0f);

        if (EggSplatTexture)
        {
            Canvas->K2_DrawTexture(
                EggSplatTexture,
                FVector2D(ScreenWidth * 0.5f - 220.0f, ScreenHeight * 0.5f - 180.0f),
                FVector2D(440.0f, 360.0f),
                FVector2D::ZeroVector,
                FVector2D(1.0f, 1.0f),
                FLinearColor(1.0f, 1.0f, 1.0f, Alpha),
                BLEND_Translucent,
                0.0f,
                FVector2D(0.5f, 0.5f));
        }
        else
        {
            const FLinearColor EggColor(1.0f, 0.72f, 0.05f, Alpha * 0.30f);
            DrawRect(EggColor, ScreenWidth * 0.15f, ScreenHeight * 0.25f, 155.0f, 95.0f);
            DrawRect(EggColor, ScreenWidth * 0.63f, ScreenHeight * 0.18f, 120.0f, 155.0f);
            DrawRect(EggColor, ScreenWidth * 0.45f, ScreenHeight * 0.68f, 190.0f, 80.0f);
        }
    }
}
