#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class AController;
class UDamageType;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInAndOutHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInAndOutDamaged, float, ActualDamage, AActor*, DamageCauser, AController*, InstigatorController);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInAndOutDeath, AActor*, DamageCauser);

UCLASS(ClassGroup=(InAndOut), meta=(BlueprintSpawnableComponent))
class INANDOUT_GAME_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

    UPROPERTY(BlueprintAssignable, Category="Health")
    FOnInAndOutHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category="Health")
    FOnInAndOutDamaged OnDamaged;

    UPROPERTY(BlueprintAssignable, Category="Health")
    FOnInAndOutDeath OnDeath;

    UFUNCTION(BlueprintCallable, Category="Health")
    void ConfigureHealth(float NewMaxHealth, bool bRefill = true);

    UFUNCTION(BlueprintCallable, Category="Health")
    void ResetHealth();

    UFUNCTION(BlueprintPure, Category="Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category="Health")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintPure, Category="Health")
    float GetHealthNormalized() const;

    UFUNCTION(BlueprintPure, Category="Health")
    bool IsDead() const { return bDead; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(EditAnywhere, Category="Health", meta=(ClampMin="1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, Category="Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, Category="Health")
    bool bDead = false;

    UFUNCTION()
    void HandleAnyDamage(
        AActor* DamagedActor,
        float Damage,
        const UDamageType* DamageType,
        AController* InstigatedBy,
        AActor* DamageCauser);

    void AwardUltimateCharge(float ActualDamage, AController* InstigatedBy, AActor* DamageCauser) const;
};
