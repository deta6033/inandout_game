#include "FoamDartProjectile.h"

#include "HealthComponent.h"
#include "SonCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

AFoamDartProjectile::AFoamDartProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(6.0f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    Collision->SetNotifyRigidBodyCollision(true);
    Collision->OnComponentHit.AddDynamic(this, &AFoamDartProjectile::HandleHit);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Collision);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 2600.0f;
    ProjectileMovement->MaxSpeed = 2600.0f;
    ProjectileMovement->ProjectileGravityScale = 0.05f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    InitialLifeSpan = 5.0f;
}

void AFoamDartProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* ProjectileOwner = GetOwner())
    {
        Collision->IgnoreActorWhenMoving(ProjectileOwner, true);
    }
}

void AFoamDartProjectile::HandleHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    bool bDamagedTarget = false;

    if (OtherActor && OtherActor != this && OtherActor != GetOwner())
    {
        const float AppliedDamage = UGameplayStatics::ApplyDamage(
            OtherActor,
            Damage,
            GetInstigatorController(),
            this,
            UDamageType::StaticClass());

        const bool bTargetHasHealth =
            OtherActor->FindComponentByClass<UHealthComponent>() != nullptr;

        bDamagedTarget = bTargetHasHealth && AppliedDamage > 0.0f;

        if (bDamagedTarget)
        {
            if (ASonCharacter* Son = Cast<ASonCharacter>(GetInstigator()))
            {
                Son->NotifyHitConfirmed();
            }
        }
    }

    BP_OnImpact(Hit, bDamagedTarget);
    Destroy();
}
