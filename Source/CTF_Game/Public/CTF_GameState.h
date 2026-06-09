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
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScoreChanged);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnScoreChanged OnScoreChangedEvent;

    // --- Setters (solo servidor los llama) ---
    void SetMatchState(EMatchState NewState);
    void SetRemainingTime(float NewTime);
    void SetWinnerTeam(int32 TeamIndex);
    void AddScore(int32 TeamIndex);
    void AddPlayerToTeam(int32 TeamIndex);

    // --- Getters ---
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CTF|GameState")
    float GetRemainingTime() const { return RemainingTime; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CTF|GameState")
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

    // ✅ CAMBIO APLICADO: Agregado ReplicatedUsing para Team A
    UPROPERTY(ReplicatedUsing = OnRep_ScoreTeamA)
    int32 ScoreTeamA;

    // ✅ CAMBIO APLICADO: Agregado ReplicatedUsing para Team B
    UPROPERTY(ReplicatedUsing = OnRep_ScoreTeamB)
    int32 ScoreTeamB;

    // Cantidad de jugadores por equipo
    UPROPERTY(Replicated)
    int32 TeamASize;

    // Cantidad de jugadores por equipo
    UPROPERTY(Replicated)
    int32 TeamBSize;

    // Equipo ganador
    UPROPERTY(ReplicatedUsing = OnRep_WinnerTeam)
    int32 WinnerTeam;

    // --- RepNotify callbacks ---
    UFUNCTION()
    void OnRep_MatchState();

    UFUNCTION()
    void OnRep_RemainingTime();

    // ✅ CAMBIO APLICADO: Declaración del OnRep para Team A
    UFUNCTION()
    void OnRep_ScoreTeamA();

    // ✅ CAMBIO APLICADO: Declaración del OnRep para Team B
    UFUNCTION()
    void OnRep_ScoreTeamB();

    UFUNCTION()
    void OnRep_WinnerTeam();

    // --- UI DE FIN DE JUEGO ---
    UPROPERTY(EditDefaultsOnly, Category = "UI|Final")
    TSubclassOf<class UUserWidget> VictoriaWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Final")
    TSubclassOf<class UUserWidget> DerrotaWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Final")
    TSubclassOf<class UUserWidget> EmpateWidgetClass;
};