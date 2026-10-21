#include "EggProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

AEggProjectile::AEggProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(9.0f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    Collision->SetNotifyRigidBodyCollision(true);
    Collision->OnComponentHit.AddDynamic(this, &AEggProjectile::HandleHit);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Collision);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 1450.0f;
    ProjectileMovement->MaxSpeed = 1450.0f;
    ProjectileMovement->ProjectileGravityScale = 0.55f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    InitialLifeSpan = 6.0f;
}

void AEggProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* ProjectileOwner = GetOwner())
    {
        Collision->IgnoreActorWhenMoving(ProjectileOwner, true);
    }
}

void AEggProjectile::HandleHit(
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

        bDamagedTarget = AppliedDamage > 0.0f;
    }

    BP_OnImpact(Hit, bDamagedTarget);
    Destroy();
}
