#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NaggingTextActor.generated.h"

class USceneComponent;
class UTextRenderComponent;

UCLASS()
class INANDOUT_GAME_API ANaggingTextActor : public AActor
{
    GENERATED_BODY()

public:
    ANaggingTextActor();

    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category="Nagging")
    void InitializeNagText(const FString& InText, const FColor& InColor);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UTextRenderComponent> TextRender;

    UPROPERTY(EditAnywhere, Category="Nagging", meta=(ClampMin="0.1"))
    float LifeSeconds = 1.35f;

    UPROPERTY(EditAnywhere, Category="Nagging")
    float RiseSpeed = 55.0f;

    UPROPERTY(EditAnywhere, Category="Nagging")
    float SwayAmount = 28.0f;

    UPROPERTY(EditAnywhere, Category="Nagging")
    float SwaySpeed = 7.0f;

    UPROPERTY(EditAnywhere, Category="Nagging")
    float BaseWorldSize = 42.0f;

    float Age = 0.0f;
    float Phase = 0.0f;
    FVector SpawnLocation = FVector::ZeroVector;
    FColor BaseColor = FColor::White;
};
