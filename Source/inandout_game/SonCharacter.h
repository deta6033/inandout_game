#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SonCharacter.generated.h"

class AController;
class UCameraComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class USceneComponent;
class UHealthComponent;
class UUltimateChargeComponent;
class AFoamDartProjectile;
class AMomCharacter;

UCLASS()
class INANDOUT_GAME_API ASonCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASonCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintPure, Category="Health")
    UHealthComponent* GetHealthComponent() const { return HealthComponent; }

    UFUNCTION(BlueprintPure, Category="Ultimate")
    UUltimateChargeComponent* GetUltimateComponent() const { return UltimateComponent; }

    UFUNCTION(BlueprintPure, Category="Combat|Ammo")
    int32 GetCurrentMagazineAmmo() const { return CurrentMagazineAmmo; }

    UFUNCTION(BlueprintPure, Category="Combat|Ammo")
    int32 GetMagazineSize() const { return MagazineSize; }

    UFUNCTION(BlueprintPure, Category="Combat|Ammo")
    bool IsReloading() const { return bReloading; }

    UFUNCTION(BlueprintPure, Category="State")
    bool IsDead() const { return bDead; }

    UFUNCTION(BlueprintPure, Category="Interaction")
    FText GetInteractionPrompt() const { return InteractionPrompt; }

    UFUNCTION(BlueprintCallable, Category="Combat")
    void NotifyHitConfirmed();

    UFUNCTION(BlueprintCallable, Category="Status")
    void ApplyNaggingSlow(float SpeedMultiplier, float Duration);

protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Pillow Melee"))
    void BP_OnPillowMelee();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Foam Dart Fired"))
    void BP_OnFoamDartFired();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Reload Started"))
    void BP_OnReloadStarted();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Reload Finished"))
    void BP_OnReloadFinished();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Report Card Ultimate"))
    void BP_OnReportCardUltimate();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Son Damaged"))
    void BP_OnDamaged(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Nagging Slow Started"))
    void BP_OnNaggingSlowStarted(float Duration);

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Nagging Slow Ended"))
    void BP_OnNaggingSlowEnded();

    UFUNCTION(BlueprintImplementableEvent, Category="Presentation", meta=(DisplayName="On Son Death"))
    void BP_OnDeath();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USkeletalMeshComponent> FirstPersonArms;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USceneComponent> WeaponRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UStaticMeshComponent> FoamDartGunMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UStaticMeshComponent> PillowMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UStaticMeshComponent> ReportCardMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UUltimateChargeComponent> UltimateComponent;

    UPROPERTY(EditAnywhere, Category="Stats|Health", meta=(ClampMin="1.0"))
    float StartingMaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, Category="Stats|Movement", meta=(ClampMin="1.0"))
    float BaseWalkSpeed = 500.0f;

    UPROPERTY(EditAnywhere, Category="Stats|Ultimate", meta=(ClampMin="1.0"))
    float UltimateMaxCharge = 100.0f;

    UPROPERTY(EditAnywhere, Category="Stats|Ultimate", meta=(ClampMin="0.0"))
    float UltimateChargePerDamage = 1.25f;

    UPROPERTY(EditAnywhere, Category="Combat|Melee", meta=(ClampMin="0.0"))
    float MeleeDamage = 20.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Melee", meta=(ClampMin="1.0"))
    float MeleeRange = 175.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Melee", meta=(ClampMin="1.0"))
    float MeleeRadius = 45.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Melee", meta=(ClampMin="0.0"))
    float MeleeCooldown = 0.65f;

    UPROPERTY(EditAnywhere, Category="Combat|Ranged")
    TSubclassOf<AFoamDartProjectile> FoamDartProjectileClass;

    UPROPERTY(EditAnywhere, Category="Combat|Ranged", meta=(ClampMin="0.0"))
    float FireCooldown = 0.25f;

    UPROPERTY(EditAnywhere, Category="Combat|Ammo", meta=(ClampMin="1"))
    int32 MagazineSize = 8;

    UPROPERTY(EditAnywhere, Category="Combat|Ammo", meta=(ClampMin="0.05"))
    float ReloadDuration = 1.25f;

    UPROPERTY(EditAnywhere, Category="Combat|Ultimate", meta=(ClampMin="1.0"))
    float ReportCardRange = 1050.0f;

    UPROPERTY(EditAnywhere, Category="Combat|Ultimate", meta=(ClampMin="0.1"))
    float ReportCardStunDuration = 3.0f;

    UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin="1.0"))
    float InteractionRange = 260.0f;

    UPROPERTY(EditAnywhere, Category="Debug")
    bool bDrawAttackDebug = false;

    int32 CurrentMagazineAmmo = 8;
    bool bReloading = false;
    bool bDead = false;

    float LastMeleeTime = -1000.0f;
    float LastFireTime = -1000.0f;

    float PreSlowWalkSpeed = 0.0f;
    FText InteractionPrompt;
    TWeakObjectPtr<AActor> FocusedInteractable;

    FTimerHandle ReloadTimerHandle;
    FTimerHandle ReportCardVisualTimerHandle;
    FTimerHandle SlowTimerHandle;

    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);

    void StartJump();
    void StopJumpAction();
    void TryMeleeAttack();
    void TryFireFoamDart();
    void StartReload();
    void FinishReload();
    void TryUseReportCardUltimate();
    void HideReportCardVisual();
    void TryInteract();

    void UpdateInteractionFocus();
    AMomCharacter* FindReportCardTarget() const;
    void GetViewPoint(FVector& OutLocation, FRotator& OutRotation) const;
    bool CanUseCombatActions() const;
    void ClearNaggingSlow();

    UFUNCTION()
    void HandleDamaged(float ActualDamage, AActor* DamageCauser, AController* InstigatorController);

    UFUNCTION()
    void HandleDeath(AActor* DamageCauser);
};
