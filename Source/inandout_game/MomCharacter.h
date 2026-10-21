#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MomCharacter.generated.h"

class AController;
class UHealthComponent;
class UUltimateChargeComponent;
class AEggProjectile;
class ANaggingTextActor;
class ASonCharacter;

UCLASS()
class INANDOUT_GAME_API AMomCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMomCharacter();

    UFUNCTION(BlueprintPure, Category="Health")
    UHealthComponent* GetHealthComponent() const { return HealthComponent; }

    UFUNCTION(BlueprintPure, Category="Ultimate")
    UUltimateChargeComponent* GetUltimateComponent() const { return UltimateComponent; }

    UFUNCTION(BlueprintPure, Category="State")
    bool IsStunned() const { return bStunned; }

    UFUNCTION(BlueprintPure, Category="State")
    bool IsDead() const { return bDead; }

    UFUNCTION(BlueprintPure, Category="Combat")
    float GetMeleeRange() const { return MeleeRange; }

    UFUNCTION(BlueprintPure, Category="Combat")
    float GetRangedRange() const { return RangedRange; }

    UFUNCTION(BlueprintPure, Category="Combat")
    float GetUltimateRange() const { return UltimateRange; }

    UFUNCTION(BlueprintPure, Category="Combat")
    bool IsUltimateReady() const;

    bool TryMeleeAttack(AActor* Target);
    bool TryEggAttack(AActor* Target);
    bool TryNaggingUltimate(AActor* Target);

    UFUNCTION(BlueprintCallable, Category="Status")
    void ApplyStun(float Duration);

protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Rubber Glove Melee"))
    void BP_OnRubberGloveMelee();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Egg Thrown"))
    void BP_OnEggThrown();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Nagging Ultimate Started"))
    void BP_OnNaggingUltimateStarted(float Duration);

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Nagging Ultimate Ended"))
    void BP_OnNaggingUltimateEnded();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Mom Stunned"))
    void BP_OnStunned(float Duration);

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Mom Stun Ended"))
    void BP_OnStunEnded();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Mom Damaged"))
    void BP_OnDamaged(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Mom Death"))
    void BP_OnDeath();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UUltimateChargeComponent> UltimateComponent;

    UPROPERTY(EditAnywhere, Category="Stats|Health", meta=(ClampMin="1.0"))
    float StartingMaxHealth = 300.0f;

    UPROPERTY(EditAnywhere, Category="Stats|Movement", meta=(ClampMin="1.0"))
    float BaseWalkSpeed = 380.0f;

    UPROPERTY(EditAnywhere, Category="Stats|Ultimate", meta=(ClampMin="1.0"))
    float UltimateMaxCharge = 100.0f;

    UPROPERTY(EditAnywhere, Category="Stats|Ultimate", meta=(ClampMin="0.0"))
    float UltimateChargePerDamage = 2.5f;

    UPROPERTY(EditAnywhere, Category="Combat|Melee", meta=(ClampMin="0.0"))
    float MeleeDamage = 15.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Melee", meta=(ClampMin="1.0"))
    float MeleeRange = 185.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Melee", meta=(ClampMin="0.0"))
    float MeleeCooldown = 1.15f;

    UPROPERTY(EditAnywhere, Category="Combat|Ranged")
    TSubclassOf<AEggProjectile> EggProjectileClass;

    UPROPERTY(EditAnywhere, Category="Combat|Ranged", meta=(ClampMin="1.0"))
    float RangedRange = 1350.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Ranged", meta=(ClampMin="0.0"))
    float RangedCooldown = 2.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Ultimate", meta=(ClampMin="1.0"))
    float UltimateRange = 1000.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Ultimate", meta=(ClampMin="0.05", ClampMax="1.0"))
    float NaggingSlowMultiplier = 0.45f;

    UPROPERTY(EditAnywhere, Category="Combat|Ultimate", meta=(ClampMin="0.1"))
    float NaggingDuration = 5.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Ultimate")
    TSubclassOf<ANaggingTextActor> NaggingTextClass;

    UPROPERTY(EditAnywhere, Category="Combat|Ultimate", meta=(ClampMin="0.05"))
    float NaggingTextSpawnInterval = 0.28f;

    float LastMeleeTime = -1000.0f;
    float LastRangedTime = -1000.0f;

    bool bStunned = false;
    bool bDead = false;
    int32 NaggingSpawnsRemaining = 0;

    FTimerHandle StunTimerHandle;
    FTimerHandle NaggingSpawnTimerHandle;
    FTimerHandle NaggingEndTimerHandle;

    void ClearStun();
    void SpawnNaggingText();
    void FinishNaggingVisuals();
    bool CanAct() const;

    UFUNCTION()
    void HandleDamaged(float ActualDamage, AActor* DamageCauser, AController* InstigatorController);

    UFUNCTION()
    void HandleDeath(AActor* DamageCauser);
};
