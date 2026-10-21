#include "SonCharacter.h"

#include "EggProjectile.h"
#include "FoamDartProjectile.h"
#include "HealthComponent.h"
#include "InAndOutInteractable.h"
#include "InAndOutPlayerController.h"
#include "MomCharacter.h"
#include "UltimateChargeComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ASonCharacter::ASonCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArms"));
    FirstPersonArms->SetupAttachment(FirstPersonCamera);
    FirstPersonArms->SetOnlyOwnerSee(true);
    FirstPersonArms->SetCastShadow(false);
    FirstPersonArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
    WeaponRoot->SetupAttachment(FirstPersonCamera);
    WeaponRoot->SetRelativeLocation(FVector(35.0f, 15.0f, -20.0f));

    FoamDartGunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FoamDartGunMesh"));
    FoamDartGunMesh->SetupAttachment(WeaponRoot);
    FoamDartGunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FoamDartGunMesh->SetOnlyOwnerSee(true);
    FoamDartGunMesh->SetCastShadow(false);

    PillowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillowMesh"));
    PillowMesh->SetupAttachment(WeaponRoot);
    PillowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PillowMesh->SetOnlyOwnerSee(true);
    PillowMesh->SetCastShadow(false);
    PillowMesh->SetVisibility(false);

    ReportCardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ReportCardMesh"));
    ReportCardMesh->SetupAttachment(WeaponRoot);
    ReportCardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ReportCardMesh->SetOnlyOwnerSee(true);
    ReportCardMesh->SetCastShadow(false);
    ReportCardMesh->SetVisibility(false);

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    UltimateComponent = CreateDefaultSubobject<UUltimateChargeComponent>(TEXT("UltimateComponent"));

    FoamDartProjectileClass = AFoamDartProjectile::StaticClass();
}

void ASonCharacter::BeginPlay()
{
    Super::BeginPlay();

    GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
    CurrentMagazineAmmo = MagazineSize;
    ApplyWeaponColors();
    UpdateWeaponPresentation();

    HealthComponent->ConfigureHealth(StartingMaxHealth, true);
    UltimateComponent->Configure(UltimateMaxCharge, UltimateChargePerDamage, true);

    HealthComponent->OnDamaged.AddDynamic(this, &ASonCharacter::HandleDamaged);
    HealthComponent->OnDeath.AddDynamic(this, &ASonCharacter::HandleDeath);
}

void ASonCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (IsLocallyControlled() && !bDead)
    {
        UpdateInteractionFocus();
    }
}

void ASonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    check(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ASonCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ASonCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ASonCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ASonCharacter::LookUp);

    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ASonCharacter::StartJump);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ASonCharacter::StopJumpAction);
    PlayerInputComponent->BindAction(TEXT("PrimaryAttack"), IE_Pressed, this, &ASonCharacter::TryPrimaryAttack);
    PlayerInputComponent->BindAction(TEXT("Weapon1"), IE_Pressed, this, &ASonCharacter::SelectPillow);
    PlayerInputComponent->BindAction(TEXT("Weapon2"), IE_Pressed, this, &ASonCharacter::SelectFoamDart);
    PlayerInputComponent->BindAction(TEXT("NextWeapon"), IE_Pressed, this, &ASonCharacter::CycleWeapon);
    PlayerInputComponent->BindAction(TEXT("Reload"), IE_Pressed, this, &ASonCharacter::StartReload);
    PlayerInputComponent->BindAction(TEXT("UseUltimate"), IE_Pressed, this, &ASonCharacter::TryUseReportCardUltimate);
    PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &ASonCharacter::TryInteract);
}

void ASonCharacter::MoveForward(float Value)
{
    if (!CanUseCombatActions() || FMath::IsNearlyZero(Value) || !Controller)
    {
        return;
    }

    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void ASonCharacter::MoveRight(float Value)
{
    if (!CanUseCombatActions() || FMath::IsNearlyZero(Value) || !Controller)
    {
        return;
    }

    const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
}

void ASonCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ASonCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ASonCharacter::StartJump()
{
    if (CanUseCombatActions())
    {
        Jump();
    }
}

void ASonCharacter::StopJumpAction()
{
    StopJumping();
}

bool ASonCharacter::CanUseCombatActions() const
{
    return !bDead && !HealthComponent->IsDead();
}

void ASonCharacter::GetViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
    if (Controller)
    {
        Controller->GetPlayerViewPoint(OutLocation, OutRotation);
    }
    else
    {
        GetActorEyesViewPoint(OutLocation, OutRotation);
    }
}

void ASonCharacter::TryPrimaryAttack()
{
    if (EquippedWeapon == EInAndOutWeaponSlot::Pillow)
    {
        TryMeleeAttack();
    }
    else
    {
        TryFireFoamDart();
    }
}

void ASonCharacter::SelectPillow()
{
    EquipWeapon(EInAndOutWeaponSlot::Pillow);
}

void ASonCharacter::SelectFoamDart()
{
    EquipWeapon(EInAndOutWeaponSlot::FoamDart);
}

void ASonCharacter::CycleWeapon()
{
    EquipWeapon(EquippedWeapon == EInAndOutWeaponSlot::Pillow
        ? EInAndOutWeaponSlot::FoamDart : EInAndOutWeaponSlot::Pillow);
}

void ASonCharacter::EquipWeapon(EInAndOutWeaponSlot NewWeapon)
{
    if (!CanUseCombatActions() || EquippedWeapon == NewWeapon)
    {
        return;
    }
    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastWeaponSwitchTime < WeaponSwitchCooldown)
    {
        return;
    }

    EquippedWeapon = NewWeapon;
    LastWeaponSwitchTime = Now;
    UpdateWeaponPresentation();
    BP_OnWeaponEquipped(EquippedWeapon);
}

void ASonCharacter::UpdateWeaponPresentation()
{
    PillowMesh->SetVisibility(EquippedWeapon == EInAndOutWeaponSlot::Pillow);
    FoamDartGunMesh->SetVisibility(EquippedWeapon == EInAndOutWeaponSlot::FoamDart);
}

void ASonCharacter::ApplyWeaponColors()
{
    // If the current material has no TintColor parameter, configure a compatible
    // Material Instance in BP_Son; changing this value alone cannot recolor it.
    auto TintMesh = [this](UStaticMeshComponent* Mesh, const FLinearColor& Tint)
    {
        if (!Mesh)
        {
            return;
        }
        for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
        {
            if (UMaterialInstanceDynamic* MID = Mesh->CreateDynamicMaterialInstance(Index))
            {
                MID->SetVectorParameterValue(WeaponTintParameterName, Tint);
            }
        }
    };
    TintMesh(PillowMesh, PillowTint);
    TintMesh(FoamDartGunMesh, FoamDartTint);
}

void ASonCharacter::TryMeleeAttack()
{
    if (!CanUseCombatActions() || EquippedWeapon != EInAndOutWeaponSlot::Pillow)
    {
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastMeleeTime < MeleeCooldown)
    {
        return;
    }

    LastMeleeTime = Now;
    BP_OnPillowMelee();

    FVector ViewLocation;
    FRotator ViewRotation;
    GetViewPoint(ViewLocation, ViewRotation);

    const FVector Start = ViewLocation;
    const FVector End = Start + ViewRotation.Vector() * MeleeRange;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SonPillowMelee), false, this);
    FHitResult Hit;

    const bool bHit = GetWorld()->SweepSingleByChannel(
        Hit,
        Start,
        End,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(MeleeRadius),
        QueryParams);

    if (bDrawAttackDebug)
    {
        DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 2.0f);
        DrawDebugSphere(GetWorld(), bHit ? Hit.ImpactPoint : End, MeleeRadius, 16, bHit ? FColor::Green : FColor::Red, false, 1.0f);
    }

    if (bHit && Hit.GetActor() && Hit.GetActor() != this)
    {
        const bool bTargetHasHealth =
            Hit.GetActor()->FindComponentByClass<UHealthComponent>() != nullptr;

        const float AppliedDamage = UGameplayStatics::ApplyDamage(
            Hit.GetActor(),
            MeleeDamage,
            Controller,
            this,
            UDamageType::StaticClass());

        if (bTargetHasHealth && AppliedDamage > 0.0f)
        {
            NotifyHitConfirmed();
        }
    }
}

void ASonCharacter::TryFireFoamDart()
{
    if (!CanUseCombatActions() || bReloading || EquippedWeapon != EInAndOutWeaponSlot::FoamDart)
    {
        return;
    }

    if (CurrentMagazineAmmo <= 0)
    {
        StartReload();
        return;
    }

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastFireTime < FireCooldown || !FoamDartProjectileClass)
    {
        return;
    }

    FVector ViewLocation;
    FRotator ViewRotation;
    GetViewPoint(ViewLocation, ViewRotation);

    const FVector SpawnLocation = ViewLocation
        + ViewRotation.Vector() * 90.0f
        + FRotationMatrix(ViewRotation).GetUnitAxis(EAxis::Y) * 12.0f
        - FRotationMatrix(ViewRotation).GetUnitAxis(EAxis::Z) * 8.0f;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AFoamDartProjectile* Projectile = GetWorld()->SpawnActor<AFoamDartProjectile>(
        FoamDartProjectileClass,
        SpawnLocation,
        ViewRotation,
        SpawnParams);

    if (!Projectile)
    {
        return;
    }

    LastFireTime = Now;
    CurrentMagazineAmmo = FMath::Max(0, CurrentMagazineAmmo - 1);
    BP_OnFoamDartFired();

    if (CurrentMagazineAmmo <= 0)
    {
        StartReload();
    }
}

void ASonCharacter::StartReload()
{
    if (!CanUseCombatActions() || bReloading || EquippedWeapon != EInAndOutWeaponSlot::FoamDart || CurrentMagazineAmmo >= MagazineSize)
    {
        return;
    }

    bReloading = true;
    BP_OnReloadStarted();

    GetWorldTimerManager().SetTimer(
        ReloadTimerHandle,
        this,
        &ASonCharacter::FinishReload,
        ReloadDuration,
        false);
}

void ASonCharacter::FinishReload()
{
    if (bDead)
    {
        return;
    }

    bReloading = false;
    CurrentMagazineAmmo = MagazineSize;
    BP_OnReloadFinished();
}

AMomCharacter* ASonCharacter::FindReportCardTarget() const
{
    FVector ViewLocation;
    FRotator ViewRotation;
    GetViewPoint(ViewLocation, ViewRotation);

    AMomCharacter* BestTarget = nullptr;
    float BestDistanceSquared = FMath::Square(ReportCardRange);

    for (TActorIterator<AMomCharacter> It(GetWorld()); It; ++It)
    {
        AMomCharacter* Candidate = *It;
        if (!Candidate || Candidate->IsDead())
        {
            continue;
        }

        const FVector ToTarget = Candidate->GetActorLocation() - ViewLocation;
        const float DistanceSquared = ToTarget.SizeSquared();
        if (DistanceSquared > BestDistanceSquared)
        {
            continue;
        }

        const FVector DirectionToTarget = ToTarget.GetSafeNormal();
        if (FVector::DotProduct(ViewRotation.Vector(), DirectionToTarget) < 0.25f)
        {
            continue;
        }

        FHitResult VisibilityHit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(ReportCardLineOfSight), false, this);
        const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
            VisibilityHit,
            ViewLocation,
            Candidate->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f),
            ECC_Visibility,
            Params);

        if (bBlocked && VisibilityHit.GetActor() != Candidate)
        {
            continue;
        }

        BestDistanceSquared = DistanceSquared;
        BestTarget = Candidate;
    }

    return BestTarget;
}

void ASonCharacter::TryUseReportCardUltimate()
{
    if (!CanUseCombatActions() || !UltimateComponent->IsReady())
    {
        return;
    }

    AMomCharacter* TargetMom = FindReportCardTarget();
    if (!TargetMom)
    {
        return;
    }

    if (!UltimateComponent->ConsumeFullCharge())
    {
        return;
    }

    TargetMom->ApplyStun(ReportCardStunDuration);

    ReportCardMesh->SetVisibility(true);
    BP_OnReportCardUltimate();

    GetWorldTimerManager().SetTimer(
        ReportCardVisualTimerHandle,
        this,
        &ASonCharacter::HideReportCardVisual,
        0.85f,
        false);
}

void ASonCharacter::HideReportCardVisual()
{
    ReportCardMesh->SetVisibility(false);
}

void ASonCharacter::UpdateInteractionFocus()
{
    FVector ViewLocation;
    FRotator ViewRotation;
    GetViewPoint(ViewLocation, ViewRotation);

    const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * InteractionRange;

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(SonInteractionFocus), false, this);
    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, TraceEnd, ECC_Visibility, Params);

    AActor* HitActor = bHit ? Hit.GetActor() : nullptr;
    if (HitActor && HitActor->GetClass()->ImplementsInterface(UInAndOutInteractable::StaticClass()))
    {
        FocusedInteractable = HitActor;
        InteractionPrompt = IInAndOutInteractable::Execute_GetInteractionText(HitActor);
    }
    else
    {
        FocusedInteractable.Reset();
        InteractionPrompt = FText::GetEmpty();
    }
}

void ASonCharacter::TryInteract()
{
    if (!CanUseCombatActions())
    {
        return;
    }

    if (FocusedInteractable.IsValid()
        && FocusedInteractable->GetClass()->ImplementsInterface(UInAndOutInteractable::StaticClass()))
    {
        IInAndOutInteractable::Execute_Interact(FocusedInteractable.Get(), this);
    }
}

void ASonCharacter::NotifyHitConfirmed()
{
    if (AInAndOutPlayerController* PC = Cast<AInAndOutPlayerController>(Controller))
    {
        PC->NotifyHitMarker();
    }
}

void ASonCharacter::ApplyNaggingSlow(float SpeedMultiplier, float Duration)
{
    if (bDead || Duration <= 0.0f)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(SlowTimerHandle);

    if (PreSlowWalkSpeed <= 0.0f)
    {
        PreSlowWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    }

    GetCharacterMovement()->MaxWalkSpeed = PreSlowWalkSpeed * FMath::Clamp(SpeedMultiplier, 0.05f, 1.0f);
    BP_OnNaggingSlowStarted(Duration);

    GetWorldTimerManager().SetTimer(
        SlowTimerHandle,
        this,
        &ASonCharacter::ClearNaggingSlow,
        Duration,
        false);
}

void ASonCharacter::ClearNaggingSlow()
{
    if (PreSlowWalkSpeed > 0.0f && !bDead)
    {
        GetCharacterMovement()->MaxWalkSpeed = PreSlowWalkSpeed;
    }

    PreSlowWalkSpeed = 0.0f;
    BP_OnNaggingSlowEnded();
}

void ASonCharacter::HandleDamaged(float ActualDamage, AActor* DamageCauser, AController* InstigatorController)
{
    BP_OnDamaged(ActualDamage, DamageCauser);

    if (AInAndOutPlayerController* PC = Cast<AInAndOutPlayerController>(Controller))
    {
        PC->NotifyPlayerDamaged();

        if (DamageCauser && DamageCauser->IsA<AEggProjectile>())
        {
            PC->NotifyEggSplat();
        }
    }
}

void ASonCharacter::HandleDeath(AActor* DamageCauser)
{
    if (bDead)
    {
        return;
    }

    bDead = true;
    bReloading = false;
    InteractionPrompt = FText::GetEmpty();

    GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
    GetWorldTimerManager().ClearTimer(ReportCardVisualTimerHandle);
    GetWorldTimerManager().ClearTimer(SlowTimerHandle);

    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    BP_OnDeath();
}
