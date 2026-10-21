#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FoamDartProjectile.generated.h"

class UPrimitiveComponent;
class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class INANDOUT_GAME_API AFoamDartProjectile : public AActor
{
    GENERATED_BODY()

public:
    AFoamDartProjectile();

protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Category="Projectile", meta=(DisplayName="On Foam Dart Impact"))
    void BP_OnImpact(const FHitResult& HitResult, bool bDamagedTarget);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditAnywhere, Category="Combat", meta=(ClampMin="0.0"))
    float Damage = 8.0f;

    UFUNCTION()
    void HandleHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit);
};
