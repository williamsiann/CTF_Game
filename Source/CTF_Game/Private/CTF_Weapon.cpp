#include "CTF_Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "CTF_IceProjectile.h"
#include "Engine/World.h"

ACTF_Weapon::ACTF_Weapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// arma replciada
	bReplicates = true;
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName(TEXT("NoCollision"));
	
	RootComponent = WeaponMesh;
}

void ACTF_Weapon::Fire(const FVector& LaunchLocation, const FRotator& LaunchRotation, APawn* InstigatorPawn)
{
	// Filtro solo el servidor puede spawnear objetos replicados
	if (!HasAuthority()) return;

	if (ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = InstigatorPawn; // dueño de la bala
		
		SpawnParams.Owner = this;

		// Disparo
		GetWorld()->SpawnActor<ACTF_IceProjectile>(ProjectileClass, LaunchLocation, LaunchRotation, SpawnParams);
	}
	
}
