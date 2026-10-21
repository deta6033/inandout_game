#include "HealthComponent.h"

#include "UltimateChargeComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    bDead = false;

    if (AActor* Owner = GetOwner())
    {
        Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleAnyDamage);
    }

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AActor* Owner = GetOwner())
    {
        Owner->OnTakeAnyDamage.RemoveDynamic(this, &UHealthComponent::HandleAnyDamage);
    }

    Super::EndPlay(EndPlayReason);
}

void UHealthComponent::ConfigureHealth(float NewMaxHealth, bool bRefill)
{
    MaxHealth = FMath::Max(1.0f, NewMaxHealth);

    if (bRefill)
    {
        CurrentHealth = MaxHealth;
        bDead = false;
    }
    else
    {
        CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
        bDead = CurrentHealth <= 0.0f;
    }

    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::ResetHealth()
{
    CurrentHealth = MaxHealth;
    bDead = false;
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float UHealthComponent::GetHealthNormalized() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UHealthComponent::HandleAnyDamage(
    AActor* DamagedActor,
    float Damage,
    const UDamageType* DamageType,
    AController* InstigatedBy,
    AActor* DamageCauser)
{
    if (bDead || Damage <= 0.0f)
    {
        return;
    }

    const float ActualDamage = FMath::Min(CurrentHealth, Damage);
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    AwardUltimateCharge(ActualDamage, InstigatedBy, DamageCauser);

    OnDamaged.Broadcast(ActualDamage, DamageCauser, InstigatedBy);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        bDead = true;
        OnDeath.Broadcast(DamageCauser);
    }
}

void UHealthComponent::AwardUltimateCharge(float ActualDamage, AController* InstigatedBy, AActor* DamageCauser) const
{
    AActor* ChargeOwner = nullptr;

    if (InstigatedBy)
    {
        ChargeOwner = InstigatedBy->GetPawn();
    }

    if (!ChargeOwner && DamageCauser)
    {
        if (APawn* DamagePawn = Cast<APawn>(DamageCauser))
        {
            ChargeOwner = DamagePawn;
        }
        else
        {
            ChargeOwner = DamageCauser->GetInstigator();
        }
    }

    if (!ChargeOwner || ChargeOwner == GetOwner())
    {
        return;
    }

    if (UUltimateChargeComponent* Ultimate = ChargeOwner->FindComponentByClass<UUltimateChargeComponent>())
    {
        Ultimate->AddChargeFromDamage(ActualDamage);
    }
}
