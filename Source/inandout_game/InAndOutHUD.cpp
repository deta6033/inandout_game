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
        if (!It->IsDead()) { Mom = *It; break; }
    }

    // Responsive gameplay HUD: compact, readable panels without new UMG assets.
    const float Scale = FMath::Clamp(FMath::Min(ScreenWidth / 1920.0f, ScreenHeight / 1080.0f), 0.70f, 1.25f);
    const float Margin = 28.0f * Scale;
    const float PanelWidth = 360.0f * Scale;
    const float PanelHeight = 137.0f * Scale;
    const float Bottom = ScreenHeight - Margin;
    const FLinearColor Panel(0.018f, 0.027f, 0.045f, 0.82f);
    const FLinearColor Muted(0.67f, 0.76f, 0.86f, 1.0f);
    const FLinearColor Accent(0.20f, 0.82f, 0.96f, 1.0f);

    if (Son)
    {
        const UHealthComponent* SonHealth = Son->GetHealthComponent();
        const UUltimateChargeComponent* SonUltimate = Son->GetUltimateComponent();
        const bool bPillow = Son->GetEquippedWeapon() == EInAndOutWeaponSlot::Pillow;
        const float LeftX = Margin;
        const float LeftY = Bottom - PanelHeight;

        DrawRect(Panel, LeftX, LeftY, PanelWidth, PanelHeight);
        DrawRect(Accent, LeftX, LeftY, 4.0f * Scale, PanelHeight);
        DrawText(TEXT("PLAYER"), Accent, LeftX + 17.0f * Scale, LeftY + 10.0f * Scale, Font, 0.95f * Scale, false);
        DrawText(TEXT("체력"), FLinearColor::White, LeftX + 17.0f * Scale, LeftY + 35.0f * Scale, Font, 0.90f * Scale, false);
        if (SonHealth)
        {
            DrawBar(LeftX + 76.0f * Scale, LeftY + 40.0f * Scale,
                264.0f * Scale, 17.0f * Scale, SonHealth->GetHealthNormalized(),
                FLinearColor(0.12f, 0.80f, 0.92f, 1.0f));
        }
        DrawText(TEXT("궁극기 [Q]"), Muted, LeftX + 17.0f * Scale, LeftY + 78.0f * Scale, Font, 0.85f * Scale, false);
        if (SonUltimate)
        {
            DrawBar(LeftX + 115.0f * Scale, LeftY + 83.0f * Scale,
                225.0f * Scale, 12.0f * Scale, SonUltimate->GetChargeNormalized(),
                FLinearColor(0.76f, 0.43f, 1.0f, 1.0f));
        }
        DrawText(TEXT("WASD 이동   |   E 상호작용"), Muted,
            LeftX + 17.0f * Scale, LeftY + 108.0f * Scale, Font, 0.73f * Scale, false);

        // Two clear weapon cards replace the narrow floating bars.
        const float RightX = ScreenWidth - Margin - PanelWidth;
        const float RightY = Bottom - PanelHeight;
        DrawRect(Panel, RightX, RightY, PanelWidth, PanelHeight);
        DrawText(TEXT("WEAPON"), Accent, RightX + 15.0f * Scale,
            RightY + 10.0f * Scale, Font, 0.95f * Scale, false);

        const float CardY = RightY + 37.0f * Scale;
        const float CardWidth = 160.0f * Scale;
        const float CardHeight = 48.0f * Scale;
        const float PillowX = RightX + 15.0f * Scale;
        const float GunX = RightX + 185.0f * Scale;
        const FLinearColor Inactive(0.10f, 0.15f, 0.20f, 0.95f);
        const FLinearColor Active(0.15f, 0.48f, 0.65f, 0.98f);
        DrawRect(bPillow ? Active : Inactive, PillowX, CardY, CardWidth, CardHeight);
        DrawRect(!bPillow ? Active : Inactive, GunX, CardY, CardWidth, CardHeight);
        DrawText(TEXT("[1] 베개"), FLinearColor::White, PillowX + 12.0f * Scale,
            CardY + 13.0f * Scale, Font, 0.96f * Scale, false);
        DrawText(TEXT("[2] 폼다트"), FLinearColor::White, GunX + 10.0f * Scale,
            CardY + 13.0f * Scale, Font, 0.90f * Scale, false);

        const FString AmmoText = bPillow
            ? TEXT("근접 공격  |  좌클릭")
            : Son->IsReloading()
                ? TEXT("재장전 중...")
                : FString::Printf(TEXT("탄약 %d / %d  |  [R] 재장전"),
                    Son->GetCurrentMagazineAmmo(), Son->GetMagazineSize());
        DrawText(AmmoText, Son->IsReloading() ? FLinearColor(1.0f, 0.80f, 0.30f, 1.0f) : FLinearColor::White,
            RightX + 15.0f * Scale, RightY + 99.0f * Scale, Font, 0.88f * Scale, false);

        const FText Prompt = Son->GetInteractionPrompt();
        if (!Prompt.IsEmpty())
        {
            const float PromptX = ScreenWidth * 0.5f - 195.0f * Scale;
            const float PromptY = ScreenHeight * 0.63f;
            DrawRect(Panel, PromptX, PromptY, 390.0f * Scale, 42.0f * Scale);
            DrawText(FString::Printf(TEXT("[E] %s"), *Prompt.ToString()),
                FLinearColor::White, PromptX + 16.0f * Scale,
                PromptY + 10.0f * Scale, Font, 0.98f * Scale, false);
        }
    }

    if (Mom)
    {
        const UHealthComponent* MomHealth = Mom->GetHealthComponent();
        const UUltimateChargeComponent* MomUltimate = Mom->GetUltimateComponent();
        const float BossWidth = FMath::Min(560.0f * Scale, ScreenWidth * 0.57f);
        const float BossX = (ScreenWidth - BossWidth) * 0.5f;
        const float BossY = 24.0f * Scale;
        DrawRect(Panel, BossX - 14.0f * Scale, BossY, BossWidth + 28.0f * Scale, 88.0f * Scale);
        DrawText(TEXT("BOSS  |  엄마"), FLinearColor(1.0f, 0.43f, 0.48f, 1.0f),
            BossX, BossY + 7.0f * Scale, Font, 1.05f * Scale, false);
        if (MomHealth)
        {
            DrawBar(BossX, BossY + 35.0f * Scale, BossWidth, 19.0f * Scale,
                MomHealth->GetHealthNormalized(), FLinearColor(0.95f, 0.19f, 0.30f, 1.0f));
        }
        if (MomUltimate)
        {
            DrawBar(BossX, BossY + 62.0f * Scale, BossWidth, 9.0f * Scale,
                MomUltimate->GetChargeNormalized(), FLinearColor(1.0f, 0.73f, 0.21f, 1.0f));
        }
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
