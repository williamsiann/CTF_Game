#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CTF_GameMode.generated.h"

UCLASS()
class CTF_GAME_API ACTF_GameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACTF_GameMode();

    // Llamado cuando un equipo captura la bandera
    void OnFlagCaptured(int32 TeamIndex);

    // Llamado cuando un jugador se conecta
    virtual void PostLogin(APlayerController* NewPlayer) override;

    // Inicia la partida
    void StartMatch();

    // Termina la partida
    void EndMatch(int32 WinnerTeam);

    // Puntos necesarios para ganar (por defecto 3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CTF Rules")
    int32 MaxScoreToWin = 3;

    int32 NextTeamToAssign = 0;

protected:
    // Puntaje necesario para ganar
    UPROPERTY(EditDefaultsOnly, Category = "CTF Rules")
    int32 ScoreToWin;

    // Tiempo límite en segundos
    UPROPERTY(EditDefaultsOnly, Category = "CTF Rules")
    float MatchTime;

    // Asigna equipo al jugador que se conecta
    void AssignTeam(APlayerController* NewPlayer);

    // Chequea si alguien ganó
    void CheckVictoryCondition(int32 TeamIndex);

    // Timer handle para el tiempo de partida
    FTimerHandle MatchTimerHandle;

    // Tick del timer
    void OnMatchTimerTick();

    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};