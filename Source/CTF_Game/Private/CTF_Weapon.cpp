#include "CTF_Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "CTF_IceProjectile.h"
#include "Engine/World.h"

ACTF_Weapon::ACTF_Weapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// Clave multijugador: El arma tiene que replicarse para que los demás vean qué tenés en la mano
	bReplicates = true;

	// Creamos la malla de la pistola sin colisión física para que no empuje al jugador al correr
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	RootComponent = WeaponMesh;
}

void ACTF_Weapon::Fire(const FVector& LaunchLocation, const FRotator& LaunchRotation, APawn* InstigatorPawn)
{
	// Filtro de seguridad absoluto: Solo el servidor puede spawnear objetos replicados en red
	if (!HasAuthority()) return;

	if (ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = InstigatorPawn; // Le pasamos el dueño de la bala
		SpawnParams.Owner = this;

		// La línea mágica: Nace el proyectil de hielo en el servidor y viaja por la red
		GetWorld()->SpawnActor<ACTF_IceProjectile>(ProjectileClass, LaunchLocation, LaunchRotation, SpawnParams);
	}
}
