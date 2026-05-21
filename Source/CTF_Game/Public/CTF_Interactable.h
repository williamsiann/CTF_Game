#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CTF_Interactable.generated.h"

UINTERFACE(MinimalAPI)
class UCTF_Interactable : public UInterface
{
	GENERATED_BODY()
};

class CTF_GAME_API ICTF_Interactable
{
	GENERATED_BODY()

public:
	// Llamado cuando un jugador interactúa con este objeto
	virtual void OnInteract(AActor* Interactor) = 0;

	// Llamado para saber si el jugador puede interactuar
	virtual bool CanInteract(AActor* Interactor) const = 0;

	// La orden para soltar
	virtual void OnDropped() = 0;
};

