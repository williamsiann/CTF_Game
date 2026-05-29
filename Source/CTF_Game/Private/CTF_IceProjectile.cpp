#include "CTF_IceProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "CTF_Damageable.h" // Incluimos tu interfaz para el filtro
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

ACTF_IceProjectile::ACTF_IceProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Le avisamos al motor que este objeto existe en la red
	bReplicates = true;
	// Le avisamos que su movimiento (el vuelo de la bala) se tiene que sincronizar
	SetReplicateMovement(true);

	// 1. Configuramos la esfera de colisión
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic")); // Bloquea y choca contra todo
	RootComponent = CollisionComponent;

	// 2. Configuramos el comportamiento de vuelo
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3000.f; // Velocidad de salida
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true; // Hielo que rebota en las paredes

	// 3. Limpieza automática: el proyectil se autodestruye a los 3 segundos de nacer
	InitialLifeSpan = 3.0f; 
}

void ACTF_IceProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Filtro de seguridad: Que haya chocado con algo válido, y que no sea Orion mismo (el que disparó)
	if (Other && Other != GetInstigator())
	{
		// MAGIA LIMPIA: En vez de un Cast To, preguntamos por la interfaz
		if (Other->Implements<UCTF_Damageable>())
		{
			// ¡Acá programaremos la lógica de congelar al enemigo!
			if (GEngine) 
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("¡Impacto de hielo! Enemigo detectado."));
			}
		}

		// Se rompe el cubito de hielo al chocar contra lo que sea
		Destroy(); 
	}
}