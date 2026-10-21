#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UltimateChargeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUltimateChargeChanged, float, CurrentCharge, float, MaxCharge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUltimateReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUltimateConsumed);

UCLASS(ClassGroup=(InAndOut), meta=(BlueprintSpawnableComponent))
class INANDOUT_GAME_API UUltimateChargeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UUltimateChargeComponent();

    UPROPERTY(BlueprintAssignable, Category="Ultimate")
    FOnUltimateChargeChanged OnChargeChanged;

    UPROPERTY(BlueprintAssignable, Category="Ultimate")
    FOnUltimateReady OnUltimateReady;

    UPROPERTY(BlueprintAssignable, Category="Ultimate")
    FOnUltimateConsumed OnUltimateConsumed;

    UFUNCTION(BlueprintCallable, Category="Ultimate")
    void Configure(float NewMaxCharge, float NewChargePerDamage, bool bResetCharge = true);

    UFUNCTION(BlueprintCallable, Category="Ultimate")
    void AddChargeFromDamage(float DamageDealt);

    UFUNCTION(BlueprintCallable, Category="Ultimate")
    bool ConsumeFullCharge();

    UFUNCTION(BlueprintCallable, Category="Ultimate")
    void ResetCharge();

    UFUNCTION(BlueprintPure, Category="Ultimate")
    float GetCurrentCharge() const { return CurrentCharge; }

    UFUNCTION(BlueprintPure, Category="Ultimate")
    float GetMaxCharge() const { return MaxCharge; }

    UFUNCTION(BlueprintPure, Category="Ultimate")
    float GetChargeNormalized() const;

    UFUNCTION(BlueprintPure, Category="Ultimate")
    bool IsReady() const { return CurrentCharge >= MaxCharge; }

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, Category="Ultimate", meta=(ClampMin="1.0"))
    float MaxCharge = 100.0f;

    UPROPERTY(EditAnywhere, Category="Ultimate", meta=(ClampMin="0.0"))
    float ChargePerDamage = 1.0f;

    UPROPERTY(VisibleAnywhere, Category="Ultimate")
    float CurrentCharge = 0.0f;
};
