#include "NaggingTextActor.h"

#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"

ANaggingTextActor::ANaggingTextActor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    TextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender"));
    TextRender->SetupAttachment(SceneRoot);
    TextRender->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    TextRender->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    TextRender->SetWorldSize(BaseWorldSize);
    TextRender->SetTextRenderColor(FColor::White);
    TextRender->SetCastShadow(false);
    TextRender->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ANaggingTextActor::BeginPlay()
{
    Super::BeginPlay();

    SpawnLocation = GetActorLocation();
    Phase = FMath::FRandRange(0.0f, 2.0f * PI);
}

void ANaggingTextActor::InitializeNagText(const FString& InText, const FColor& InColor)
{
    BaseColor = InColor;
    TextRender->SetText(FText::FromString(InText));
    TextRender->SetTextRenderColor(BaseColor);
    TextRender->SetWorldSize(BaseWorldSize * FMath::FRandRange(0.85f, 1.2f));
}

void ANaggingTextActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    Age += DeltaSeconds;
    const float Alpha = FMath::Clamp(Age / LifeSeconds, 0.0f, 1.0f);

    FVector NewLocation = SpawnLocation;
    NewLocation.Z += RiseSpeed * Age;
    NewLocation.X += FMath::Sin(Age * SwaySpeed + Phase) * SwayAmount;
    NewLocation.Y += FMath::Cos(Age * (SwaySpeed * 0.83f) + Phase) * (SwayAmount * 0.65f);
    SetActorLocation(NewLocation);

    const float Pulse = 1.0f + FMath::Sin(Age * 12.0f + Phase) * 0.12f;
    SetActorScale3D(FVector(Pulse));

    FColor FadedColor = BaseColor;
    FadedColor.A = static_cast<uint8>(255.0f * (1.0f - Alpha));
    TextRender->SetTextRenderColor(FadedColor);

    if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
    {
        const FVector ToCamera = CameraManager->GetCameraLocation() - GetActorLocation();
        FRotator FacingRotation = ToCamera.Rotation();
        FacingRotation.Yaw += 180.0f;
        SetActorRotation(FacingRotation);
    }

    if (Age >= LifeSeconds)
    {
        Destroy();
    }
}
