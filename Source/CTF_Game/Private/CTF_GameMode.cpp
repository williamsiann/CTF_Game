#include "CTF_GameMode.h"
#include "CTF_GameState.h"
#include "CTF_PlayerState.h"
#include "CTF_GamePlayerController.h"   // ← Cambiado: usamos el controller del proyecto
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "CTF_GameCharacter.h"

ACTF_GameMode::ACTF_GameMode()
{
    ScoreToWin = 3;
    MatchTime = 300.f; // 5 minutos
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

    FString LoginMsg = FString::Printf(
        TEXT("¡Jugador %s asignado al EQUIPO %d!"), *NewPlayer->GetName(), AssignedTeam);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LoginMsg);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, LoginMsg);
    }
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
        int32 Winner = GS->GetLeadingTeam();
        EndMatch(Winner);
    }
}

void ACTF_GameMode::OnFlagCaptured(int32 TeamIndex)
{
    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    GS->AddScore(TeamIndex);

    if (GEngine)
    {
        FString ScoreMsg = FString::Printf(
            TEXT("MARCADOR GLOBAL ➔ Equipo 0: %d | Equipo 1: %d"),
            GS->GetTeamScore(0), GS->GetTeamScore(1));
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, ScoreMsg);
    }

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
    GS->SetWinnerTeam(WinnerTeam);
    

    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);

    // ← Cambiado: casteamos a ACTF_GamePlayerController
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ACTF_GamePlayerController* PC = Cast<ACTF_GamePlayerController>(It->Get());
        if (PC)
        {
            PC->OnMatchEnded(WinnerTeam);
        }
    }
}