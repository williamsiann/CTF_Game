#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CTF_CaptureBase.generated.h"

UCLASS()
class CTF_GAME_API ACTF_CaptureBase : public AActor
{
	GENERATED_BODY()
    
public: 
	ACTF_CaptureBase();

	// Evento nativo de colisión
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	// Define de qué equipo es esta base (0 = Equipo A, 1 = Equipo B)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CTF Rules")
	int32 TeamIndex;

protected:
	// La zona donde el jugador debe entrar para anotar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CaptureZone;
};