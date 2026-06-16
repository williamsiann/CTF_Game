#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CTF_GameMode.generated.h"

UENUM(BlueprintType)
enum class EMatchEndReason : uint8
{
    TeamWon     UMETA(DisplayName = "Team Won"),
    Draw        UMETA(DisplayName = "Draw - Time Up")
};

UCLASS()
class CTF_GAME_API ACTF_GameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACTF_GameMode();
    
    void OnFlagCaptured(int32 TeamIndex);
    
    virtual void PostLogin(APlayerController* NewPlayer) override;
    
    void StartMatch();
    
    void EndMatch(int32 WinnerTeam);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CTF Rules")
    int32 MaxScoreToWin = 3;

    int32 NextTeamToAssign = 0;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "CTF Rules")
    int32 ScoreToWin;
    
    UPROPERTY(EditDefaultsOnly, Category = "CTF Rules")
    float MatchTime;
    
    void AssignTeam(APlayerController* NewPlayer);
    
    void CheckVictoryCondition(int32 TeamIndex);
    
    FTimerHandle MatchTimerHandle;
    
    void OnMatchTimerTick();

    virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

    UPROPERTY(BlueprintReadOnly, Category = "CTF Rules")
    EMatchEndReason MatchEndReason;

    virtual void BeginPlay() override;
};