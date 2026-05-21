#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CTF_GameState.generated.h"

// Enum para el estado de la partida
UENUM(BlueprintType)
enum class EMatchState : uint8
{
	WaitingToStart  UMETA(DisplayName = "Waiting To Start"),
	InProgress      UMETA(DisplayName = "In Progress"),
	GameOver        UMETA(DisplayName = "Game Over")
};

UCLASS()
class CTF_GAME_API ACTF_GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ACTF_GameState();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Setters (solo servidor los llama) ---
	void SetMatchState(EMatchState NewState);
	void SetRemainingTime(float NewTime);
	void SetWinnerTeam(int32 TeamIndex);
	void AddScore(int32 TeamIndex);
	void AddPlayerToTeam(int32 TeamIndex);

	// --- Getters ---
	float GetRemainingTime() const { return RemainingTime; }
	int32 GetTeamScore(int32 TeamIndex) const;
	int32 GetTeamSize(int32 TeamIndex) const;
	int32 GetLeadingTeam() const;
	EMatchState GetMatchState() const { return MatchState; }

protected:

	// Estado de la partida - replicado
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	EMatchState MatchState;

	// Tiempo restante - replicado con RepNotify ✅
	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	float RemainingTime;

	// Puntajes de equipos - replicados
	UPROPERTY(Replicated)
	int32 ScoreTeamA;

	UPROPERTY(Replicated)
	int32 ScoreTeamB;

	// Cantidad de jugadores por equipo
	UPROPERTY(Replicated)
	int32 TeamASize;

	UPROPERTY(Replicated)
	int32 TeamBSize;

	// Equipo ganador
	UPROPERTY(ReplicatedUsing = OnRep_WinnerTeam)
	int32 WinnerTeam;

	// --- RepNotify callbacks ---
	UFUNCTION()
	void OnRep_MatchState();

	UFUNCTION()
	void OnRep_RemainingTime();  // ✅ RepNotify requerido

	UFUNCTION()
	void OnRep_WinnerTeam();
};