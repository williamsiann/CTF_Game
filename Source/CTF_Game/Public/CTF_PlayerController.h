#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"          // ← ANTES del generated
#include "CTF_PlayerController.generated.h"  // ← SIEMPRE AL FINAL

UCLASS()
class CTF_GAME_API ACTF_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACTF_PlayerController();

	void OnMatchEnded(int32 WinnerTeam);
	void OnMatchStarted();
	void UpdateHUDTime(float RemainingTime);
	void UpdateHUDScore(int32 ScoreA, int32 ScoreB);
	void UpdateHUDFlagStatus(bool bHasFlag);

	// Nuevo
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	class UUserWidget* HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> EndGameWidgetClass;

	void CreateHUD();

	UFUNCTION(Client, Reliable)
	void Client_ShowEndScreen(int32 WinnerTeam);

	UFUNCTION(Client, Reliable)
	void Client_CreateHUD();
};