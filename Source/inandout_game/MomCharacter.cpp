#include "MomCharacter.h"

#include "EggProjectile.h"
#include "HealthComponent.h"
#include "MomAIController.h"
#include "NaggingTextActor.h"
#include "SonCharacter.h"
#include "UltimateChargeComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMomCharacter::AMomCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    UltimateComponent = CreateDefaultSubobject<UUltimateChargeComponent>(TEXT("UltimateComponent"));

    GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 420.0f, 0.0f);

    AIControllerClass = AMomAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    EggProjectileClass = AEggProjectile::StaticClass();
    NaggingTextClass = ANaggingTextActor::StaticClass();
}

void AMomCharacter::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    HealthComponent->ConfigureHealth(StartingMaxHealth, true);
    UltimateComponent->Configure(UltimateMaxCharge, UltimateChargePerDamage, true);

    HealthComponent->OnDamaged.AddDynamic(this, &AMomCharacter::HandleDamaged);
    HealthComponent->OnDeath.AddDynamic(this, &AMomCharacter::HandleDeath);
}

bool AMomCharacter::CanAct() const
{
    return !bDead && !bStunned && !HealthComponent->IsDead();
}

bool AMomCharacter::IsUltimateReady() const
{
    return UltimateComponent && UltimateComponent->IsReady();
}

bool AMomCharacter::TryMeleeAttack(AActor* Target)
{
    if (!CanAct() || !Target)
    {
        return false;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastMeleeTime < MeleeCooldown)
    {
        return false;
    }

    if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) > MeleeRange)
    {
        return false;
    }

    LastMeleeTime = Now;
    BP_OnRubberGloveMelee();

    UGameplayStatics::ApplyDamage(
        Target,
        MeleeDamage,
        GetController(),
        this,
        UDamageType::StaticClass());

    return true;
}

bool AMomCharacter::TryEggAttack(AActor* Target)
{
    if (!CanAct() || !Target || !EggProjectileClass)
    {
        return false;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastRangedTime < RangedCooldown)
    {
        return false;
    }

    if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) > RangedRange)
    {
        return false;
    }

    const FVector SpawnLocation = GetActorLocation()
        + GetActorForwardVector() * 80.0f
        + FVector(0.0f, 0.0f, 65.0f);

    const FVector AimPoint = Target->GetActorLocation();
    const FRotator SpawnRotation = (AimPoint - SpawnLocation).Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEggProjectile* Egg = GetWorld()->SpawnActor<AEggProjectile>(
        EggProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams);

    if (!Egg)
    {
        return false;
    }

    LastRangedTime = Now;
    BP_OnEggThrown();
    return true;
}

bool AMomCharacter::TryNaggingUltimate(AActor* Target)
{
    if (!CanAct() || !Target || !UltimateComponent->IsReady())
    {
        return false;
    }

    if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) > UltimateRange)
    {
        return false;
    }

    ASonCharacter* Son = Cast<ASonCharacter>(Target);
    if (!Son)
    {
        return false;
    }

    Son->ApplyNaggingSlow(NaggingSlowMultiplier, NaggingDuration);

    if (!UltimateComponent->ConsumeFullCharge())
    {
        return false;
    }

    BP_OnNaggingUltimateStarted(NaggingDuration);

    NaggingSpawnsRemaining = FMath::Max(
        1,
        FMath::CeilToInt(NaggingDuration / FMath::Max(0.05f, NaggingTextSpawnInterval)));

    SpawnNaggingText();

    GetWorldTimerManager().SetTimer(
        NaggingSpawnTimerHandle,
        this,
        &AMomCharacter::SpawnNaggingText,
        NaggingTextSpawnInterval,
        true);

    GetWorldTimerManager().SetTimer(
        NaggingEndTimerHandle,
        this,
        &AMomCharacter::FinishNaggingVisuals,
        NaggingDuration,
        false);

    return true;
}

void AMomCharacter::SpawnNaggingText()
{
    if (bDead || NaggingSpawnsRemaining <= 0 || !NaggingTextClass)
    {
        GetWorldTimerManager().ClearTimer(NaggingSpawnTimerHandle);
        return;
    }

    static const TArray<FString> Symbols =
    {
        TEXT("@#!@#!!"),
        TEXT("!@#$%@!"),
        TEXT("#@!!*#!"),
        TEXT("@#@!?!"),
        TEXT("!!#@$#!")
    };

    const FVector RandomOffset(
        FMath::FRandRange(-95.0f, 95.0f),
        FMath::FRandRange(-95.0f, 95.0f),
        FMath::FRandRange(85.0f, 185.0f));

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ANaggingTextActor* TextActor = GetWorld()->SpawnActor<ANaggingTextActor>(
        NaggingTextClass,
        GetActorLocation() + RandomOffset,
        FRotator::ZeroRotator,
        SpawnParams);

    if (TextActor)
    {
        const FString& Text = Symbols[FMath::RandRange(0, Symbols.Num() - 1)];
        const FColor Color = FMath::RandBool()
            ? FColor(255, 80, 100, 255)
            : FColor(255, 220, 70, 255);

        TextActor->InitializeNagText(Text, Color);
    }

    --NaggingSpawnsRemaining;

    if (NaggingSpawnsRemaining <= 0)
    {
        GetWorldTimerManager().ClearTimer(NaggingSpawnTimerHandle);
    }
}

void AMomCharacter::FinishNaggingVisuals()
{
    GetWorldTimerManager().ClearTimer(NaggingSpawnTimerHandle);
    BP_OnNaggingUltimateEnded();
}

void AMomCharacter::ApplyStun(float Duration)
{
    if (bDead || Duration <= 0.0f)
    {
        return;
    }

    bStunned = true;

    if (AController* MomController = GetController())
    {
        MomController->StopMovement();
    }

    GetCharacterMovement()->StopMovementImmediately();
    BP_OnStunned(Duration);

    GetWorldTimerManager().ClearTimer(StunTimerHandle);
    GetWorldTimerManager().SetTimer(
        StunTimerHandle,
        this,
        &AMomCharacter::ClearStun,
        Duration,
        false);
}

void AMomCharacter::ClearStun()
{
    if (bDead)
    {
        return;
    }

    bStunned = false;
    BP_OnStunEnded();
}

void AMomCharacter::HandleDamaged(float ActualDamage, AActor* DamageCauser, AController* InstigatorController)
{
    BP_OnDamaged(ActualDamage, DamageCauser);
}

void AMomCharacter::HandleDeath(AActor* DamageCauser)
{
    if (bDead)
    {
        return;
    }

    bDead = true;
    bStunned = false;

    GetWorldTimerManager().ClearTimer(StunTimerHandle);
    GetWorldTimerManager().ClearTimer(NaggingSpawnTimerHandle);
    GetWorldTimerManager().ClearTimer(NaggingEndTimerHandle);

    if (AController* MomController = GetController())
    {
        MomController->StopMovement();
        MomController->UnPossess();
    }

    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    BP_OnDeath();
}
