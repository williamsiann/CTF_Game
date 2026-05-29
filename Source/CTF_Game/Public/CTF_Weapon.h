#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CTF_Weapon.generated.h"

class UStaticMeshComponent;

UCLASS()
class CTF_GAME_API ACTF_Weapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ACTF_Weapon();

	// Función principal de disparo (se ejecutará en el Servidor)
	void Fire(const FVector& LaunchLocation, const FRotator& LaunchRotation, APawn* InstigatorPawn);

protected:
	// El modelo 3D de la pistola
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;

	// La "receta" del proyectil. Nos permite elegir qué bala dispara desde el Blueprint
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Config")
	TSubclassOf<class ACTF_IceProjectile> ProjectileClass;
};