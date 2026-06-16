#include "CTF_GameMode.h"
#include "CTF_GameState.h"
#include "CTF_PlayerState.h"
#include "CTF_GamePlayerController.h"   // ← Cambiado: usamos el controller del proyecto
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "CTF_GameCharacter.h"

ACTF_GameMode::ACTF_GameMode()
{
    ScoreToWin = 3;
    MatchTime = 1200.f; 
}

void ACTF_GameMode::BeginPlay()
{
    Super::BeginPlay();
    StartMatch();
}

void ACTF_GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    AssignTeam(NewPlayer);
}

void ACTF_GameMode::AssignTeam(APlayerController* NewPlayer)
{
    if (!NewPlayer) return;

    ACTF_PlayerState* PS = NewPlayer->GetPlayerState<ACTF_PlayerState>();
    if (!PS) return;

    int32 AssignedTeam = NextTeamToAssign;
    NextTeamToAssign = (NextTeamToAssign == 0) ? 1 : 0;

    PS->SetTeam(AssignedTeam);

    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (GS)
    {
        GS->AddPlayerToTeam(AssignedTeam);
    }

    if (ACTF_GameCharacter* MiPersonaje = Cast<ACTF_GameCharacter>(NewPlayer->GetPawn()))
    {
        MiPersonaje->TeamID = AssignedTeam;
        MiPersonaje->OnRep_Team();
    }
    
    RestartPlayer(NewPlayer);
}

void ACTF_GameMode::StartMatch()
{
    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    GS->SetMatchState(EMatchState::InProgress);
    GS->SetRemainingTime(MatchTime);

    GetWorldTimerManager().SetTimer(
        MatchTimerHandle,
        this,
        &ACTF_GameMode::OnMatchTimerTick,
        1.f,
        true
    );
}

void ACTF_GameMode::OnMatchTimerTick()
{
    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    float NewTime = GS->GetRemainingTime() - 1.f;
    GS->SetRemainingTime(NewTime);

    if (NewTime <= 0.f)
    {
        MatchEndReason = EMatchEndReason::Draw;
        EndMatch(-1); // -1 = nadie gana
    }
}

void ACTF_GameMode::OnFlagCaptured(int32 TeamIndex)
{
    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    GS->AddScore(TeamIndex);

    CheckVictoryCondition(TeamIndex);
}

void ACTF_GameMode::CheckVictoryCondition(int32 TeamIndex)
{
    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    if (GS->GetTeamScore(TeamIndex) >= ScoreToWin)
    {
        EndMatch(TeamIndex);
    }
}

void ACTF_GameMode::EndMatch(int32 WinnerTeam)
{
    GetWorldTimerManager().ClearTimer(MatchTimerHandle);

    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    GS->SetMatchState(EMatchState::GameOver);
    GS->SetWinnerTeam(WinnerTeam); // -1 = empate

    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ACTF_GamePlayerController* PC = Cast<ACTF_GamePlayerController>(It->Get());
        if (PC)
        {
            PC->OnMatchEnded(WinnerTeam); 
        }
    }
}

AActor* ACTF_GameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    int32 TeamToSpawn = -1;

    if (ACTF_PlayerState* PS = Player->GetPlayerState<ACTF_PlayerState>())
    {
        TeamToSpawn = PS->GetTeam();
    }
    
    FName TargetTag = (TeamToSpawn == 0) ? FName("Team0") : FName("Team1");
    
    TArray<AActor*> Candidates;
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        APlayerStart* PS_Actor = *It;
        if (PS_Actor->PlayerStartTag == TargetTag)
        {
            Candidates.Add(PS_Actor);
        }
    }
    
    if (Candidates.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, Candidates.Num() - 1);
        return Candidates[RandomIndex];
    }
    
    return Super::ChoosePlayerStart_Implementation(Player);
}