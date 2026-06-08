// Copyright Epic Games, Inc. All Rights Reserved.

#include "CTF_GamePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "CTF_Game.h"
#include "Widgets/Input/SVirtualJoystick.h"

// ============================================================
//  Lifecycle
// ============================================================

void ACTF_GamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Solo el controlador local crea widgets
	if (!IsLocalPlayerController()) return;

	// --- Controles móviles ---
	if (ShouldUseTouchControls() && MobileControlsWidgetClass)
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogCTF_Game, Error, TEXT("No se pudo crear el widget de controles móviles."));
		}
	}

	// --- HUD ---
	// Lo creamos vía RPC para que funcione tanto en standalone como en multijugador
	Client_CreateHUD();
}

void ACTF_GamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController()) return;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}

		if (!ShouldUseTouchControls())
		{
			for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

// ============================================================
//  Helpers
// ============================================================

bool ACTF_GamePlayerController::ShouldUseTouchControls() const
{
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

// ============================================================
//  HUD / Match events  (llamados desde GameMode)
// ============================================================

void ACTF_GamePlayerController::OnMatchStarted()
{
	Client_CreateHUD();
}

void ACTF_GamePlayerController::OnMatchEnded(int32 WinnerTeam)
{
	Client_ShowEndScreen(WinnerTeam);
}

void ACTF_GamePlayerController::UpdateHUDTime(float RemainingTime)
{
	// Conectar con el widget de HUD cuando esté listo
	UE_LOG(LogTemp, Log, TEXT("Tiempo restante: %.0f"), RemainingTime);
}

void ACTF_GamePlayerController::UpdateHUDScore(int32 ScoreA, int32 ScoreB)
{
	// Conectar con el widget de HUD cuando esté listo
	UE_LOG(LogTemp, Log, TEXT("Score - Rojo: %d | Azul: %d"), ScoreA, ScoreB);
}

void ACTF_GamePlayerController::UpdateHUDFlagStatus(bool bHasFlag)
{
	// Conectar con el widget de HUD cuando esté listo
	UE_LOG(LogTemp, Log, TEXT("Tiene bandera: %s"), bHasFlag ? TEXT("Si") : TEXT("No"));
}

// ============================================================
//  Client RPCs
// ============================================================

void ACTF_GamePlayerController::Client_CreateHUD_Implementation()
{
	if (!HUDWidgetClass) return;

	// Evitamos crear el HUD dos veces si ya existe
	if (HUDWidget) return;

	HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
	}
}

void ACTF_GamePlayerController::Client_ShowEndScreen_Implementation(int32 WinnerTeam)
{
	if (!EndGameWidgetClass) return;

	UUserWidget* EndWidget = CreateWidget<UUserWidget>(this, EndGameWidgetClass);
	if (EndWidget)
	{
		EndWidget->AddToViewport();
	}

	// Mostrar cursor y bloquear input de juego
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}
