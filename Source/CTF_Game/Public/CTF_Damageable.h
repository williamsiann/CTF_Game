#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CTF_Damageable.generated.h"

UINTERFACE(MinimalAPI)
class UCTF_Damageable : public UInterface
{
	GENERATED_BODY()
};

class CTF_GAME_API ICTF_Damageable
{
	GENERATED_BODY()

public:
	// Llamado cuando el actor recibe daño
	virtual void OnDamaged(float DamageAmount, AActor* DamageCauser) = 0;

	// Llamado cuando el actor muere
	virtual void OnDeath(AActor* Killer) = 0;

	// Llamado cuando el actor hace respawn
	virtual void OnRespawn() = 0;
};