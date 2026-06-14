#include "CTF_IceProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CTF_Damageable.h" // Incluimos tu interfaz para el filtro
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

ACTF_IceProjectile::ACTF_IceProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
   
    bReplicates = true;
   
    SetReplicateMovement(true);

    // Esfera de colisión
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->InitSphereRadius(15.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic")); // Bloquea y choca contra todo
    RootComponent = CollisionComponent;

    // Comportamiento de vuelo
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComponent;
    ProjectileMovement->InitialSpeed = 3000.f; // Velocidad de salida
    ProjectileMovement->MaxSpeed = 3000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = true;

    // autodestruye 3s
    InitialLifeSpan = 3.0f; 
}

// Avisa de la colicion
void ACTF_IceProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    // que haya chocado con algo válido, y que no sea el mismo 
    if (Other && Other != GetInstigator())
    {
       // Casteo a la Interfaz
       if (ICTF_Damageable* Target = Cast<ICTF_Damageable>(Other))
       {
          // El servidor es el único que tiene autoridad para ejecutar estados de red importantes
          if (HasAuthority())
          {
             // Ejecutamos la función de la interfaz. Congelamos por 3 segundos.
             Target->OnCongelado(3.0f);
          }
       }
       Destroy(); 
    }
}