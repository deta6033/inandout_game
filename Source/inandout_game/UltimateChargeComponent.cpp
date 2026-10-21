#include "UltimateChargeComponent.h"

UUltimateChargeComponent::UUltimateChargeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UUltimateChargeComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentCharge = FMath::Clamp(CurrentCharge, 0.0f, MaxCharge);
    OnChargeChanged.Broadcast(CurrentCharge, MaxCharge);
}

void UUltimateChargeComponent::Configure(float NewMaxCharge, float NewChargePerDamage, bool bResetCharge)
{
    MaxCharge = FMath::Max(1.0f, NewMaxCharge);
    ChargePerDamage = FMath::Max(0.0f, NewChargePerDamage);

    if (bResetCharge)
    {
        CurrentCharge = 0.0f;
    }
    else
    {
        CurrentCharge = FMath::Clamp(CurrentCharge, 0.0f, MaxCharge);
    }

    OnChargeChanged.Broadcast(CurrentCharge, MaxCharge);
}

void UUltimateChargeComponent::AddChargeFromDamage(float DamageDealt)
{
    if (DamageDealt <= 0.0f || IsReady())
    {
        return;
    }

    const bool bWasReady = IsReady();
    CurrentCharge = FMath::Clamp(CurrentCharge + DamageDealt * ChargePerDamage, 0.0f, MaxCharge);
    OnChargeChanged.Broadcast(CurrentCharge, MaxCharge);

    if (!bWasReady && IsReady())
    {
        OnUltimateReady.Broadcast();
    }
}

bool UUltimateChargeComponent::ConsumeFullCharge()
{
    if (!IsReady())
    {
        return false;
    }

    CurrentCharge = 0.0f;
    OnChargeChanged.Broadcast(CurrentCharge, MaxCharge);
    OnUltimateConsumed.Broadcast();
    return true;
}

void UUltimateChargeComponent::ResetCharge()
{
    CurrentCharge = 0.0f;
    OnChargeChanged.Broadcast(CurrentCharge, MaxCharge);
}

float UUltimateChargeComponent::GetChargeNormalized() const
{
    return MaxCharge > 0.0f ? CurrentCharge / MaxCharge : 0.0f;
}
