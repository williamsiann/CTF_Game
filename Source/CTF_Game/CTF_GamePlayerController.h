// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CTF_GamePlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *  PlayerController base del proyecto.
 *  Maneja input, controles móviles, y ahora también el HUD del CTF.
 */
UCLASS(abstract)
class ACTF_GamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	// --- HUD / UI ---
	void OnMatchStarted();
	void OnMatchEnded(int32 WinnerTeam);
	void UpdateHUDTime(float RemainingTime);
	void UpdateHUDScore(int32 ScoreA, int32 ScoreB);
	void UpdateHUDFlagStatus(bool bHasFlag);

protected:

	// --- Input ---

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts excluidos en mobile */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	// --- UI ---

	/** Widget del HUD principal */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	/** Widget de pantalla final (victoria/derrota) */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> EndGameWidgetClass;

	/** Referencia viva al HUD */
	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;

	// --- Mobile ---

	/** Mobile controls widget a spawnear */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Puntero al widget de controles móviles */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Si es true, fuerza controles táctiles aunque no sea mobile */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	// --- Overrides ---
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// --- Helpers ---
	bool ShouldUseTouchControls() const;

	// --- Client RPCs ---

	UFUNCTION(Client, Reliable)
	void Client_CreateHUD();

	UFUNCTION(Client, Reliable)
	void Client_ShowEndScreen(int32 WinnerTeam);
};
