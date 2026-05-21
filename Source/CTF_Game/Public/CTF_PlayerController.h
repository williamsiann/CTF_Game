#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CTF_PlayerController.generated.h"

UCLASS()
class CTF_GAME_API ACTF_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACTF_PlayerController();

	// Llamado por el GameMode cuando termina la partida
	void OnMatchEnded(int32 WinnerTeam);

	// Llamado cuando empieza la partida
	void OnMatchStarted();

	// Actualiza el HUD con el tiempo restante
	void UpdateHUDTime(float RemainingTime);

	// Actualiza el HUD con los puntajes
	void UpdateHUDScore(int32 ScoreA, int32 ScoreB);

	// Actualiza el HUD con el estado de la bandera
	void UpdateHUDFlagStatus(bool bHasFlag);

protected:

	virtual void BeginPlay() override;

	// Referencia al widget del HUD
	UPROPERTY()
	class UUserWidget* HUDWidget;

	// Clase del widget HUD para instanciar
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	// Clase del widget de victoria/derrota
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> EndGameWidgetClass;

	// Crea y muestra el HUD
	void CreateHUD();

	// Client RPC para mostrar pantalla de fin ✅
	UFUNCTION(Client, Reliable)
	void Client_ShowEndScreen(int32 WinnerTeam);

	// Client RPC para mostrar el HUD ✅
	UFUNCTION(Client, Reliable)
	void Client_CreateHUD();
};