#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CTF_PlayerState.generated.h"

UCLASS()
class CTF_GAME_API ACTF_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACTF_PlayerState();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Setters ---
	void SetTeam(int32 NewTeam);
	void AddScore(int32 Amount);
	void SetHasFlag(bool bNewHasFlag);

	// --- Getters ---
	int32 GetTeam() const { return TeamIndex; }
	int32 GetIndividualScore() const { return IndividualScore; }
	bool GetHasFlag() const { return bHasFlag; }

protected:

	// Equipo del jugador (0 = rojo, 1 = azul)
	UPROPERTY(ReplicatedUsing = OnRep_TeamIndex)
	int32 TeamIndex;

	// Puntaje individual
	UPROPERTY(Replicated)
	int32 IndividualScore;

	// Si el jugador tiene la bandera enemiga
	UPROPERTY(ReplicatedUsing = OnRep_HasFlag)
	bool bHasFlag;

	// --- RepNotify callbacks ---
	UFUNCTION()
	void OnRep_TeamIndex();

	UFUNCTION()
	void OnRep_HasFlag();
};