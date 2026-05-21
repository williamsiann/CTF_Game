#include "CTF_PlayerController.h"
#include "Blueprint/UserWidget.h"


ACTF_PlayerController::ACTF_PlayerController()
{
	HUDWidget = nullptr;
}

void ACTF_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Solo el cliente local crea su HUD
	if (IsLocalController())
	{
		Client_CreateHUD();
	}
}

void ACTF_PlayerController::CreateHUD()
{
	if (!HUDWidgetClass) 
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidgetClass no asignada en PlayerController"));
		return;
	}

	// Por ahora lo dejamos preparado, 
	// lo conectamos cuando hagamos la UI
	UE_LOG(LogTemp, Log, TEXT("HUD creado"));
}

void ACTF_PlayerController::OnMatchStarted()
{
	Client_CreateHUD();
}

void ACTF_PlayerController::OnMatchEnded(int32 WinnerTeam)
{
	Client_ShowEndScreen(WinnerTeam);
}

void ACTF_PlayerController::UpdateHUDTime(float RemainingTime)
{
	// Lo conectamos con el widget más adelante
	UE_LOG(LogTemp, Log, TEXT("Tiempo restante: %.0f"), RemainingTime);
}

void ACTF_PlayerController::UpdateHUDScore(int32 ScoreA, int32 ScoreB)
{
	// Lo conectamos con el widget más adelante
	UE_LOG(LogTemp, Log, TEXT("Score - Rojo: %d | Azul: %d"), ScoreA, ScoreB);
}

void ACTF_PlayerController::UpdateHUDFlagStatus(bool bHasFlag)
{
	// Lo conectamos con el widget más adelante
	UE_LOG(LogTemp, Log, TEXT("Tiene bandera: %s"), bHasFlag ? TEXT("Si") : TEXT("No"));
}

// --- Client RPCs ---

void ACTF_PlayerController::Client_ShowEndScreen_Implementation(int32 WinnerTeam)
{
	if (!EndGameWidgetClass) return;

	UUserWidget* EndWidget = CreateWidget<UUserWidget>(this, EndGameWidgetClass);
	if (EndWidget)
	{
		EndWidget->AddToViewport();
	}

	// Mostrar cursor y deshabilitar input de juego
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}

void ACTF_PlayerController::Client_CreateHUD_Implementation()
{
	CreateHUD();
}