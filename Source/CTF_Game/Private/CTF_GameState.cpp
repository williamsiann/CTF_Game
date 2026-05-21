// Fill out your copyright notice in the Description page of Project Settings.

#include "CTF_GameState.h"
#include "CTF_GameMode.h"
#include "Net/UnrealNetwork.h"

ACTF_GameState::ACTF_GameState()
{
    MatchState  = EMatchState::WaitingToStart;
    RemainingTime = 300.f;
    ScoreTeamA  = 0;
    ScoreTeamB  = 0;
    TeamASize   = 0;
    TeamBSize   = 0;
    WinnerTeam  = -1; // -1 = nadie ganó todavía
}

// Registramos todas las variables replicadas
void ACTF_GameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACTF_GameState, MatchState);
    DOREPLIFETIME(ACTF_GameState, RemainingTime);
    DOREPLIFETIME(ACTF_GameState, ScoreTeamA);
    DOREPLIFETIME(ACTF_GameState, ScoreTeamB);
    DOREPLIFETIME(ACTF_GameState, TeamASize);
    DOREPLIFETIME(ACTF_GameState, TeamBSize);
    DOREPLIFETIME(ACTF_GameState, WinnerTeam);
}

// --- Setters ---

void ACTF_GameState::SetMatchState(EMatchState NewState)
{
    MatchState = NewState;
    OnRep_MatchState(); // llamamos manual en servidor
}

void ACTF_GameState::SetRemainingTime(float NewTime)
{
    RemainingTime = NewTime;
    OnRep_RemainingTime(); // llamamos manual en servidor
}

void ACTF_GameState::SetWinnerTeam(int32 TeamIndex)
{
    WinnerTeam = TeamIndex;
    OnRep_WinnerTeam();
}

void ACTF_GameState::AddScore(int32 TeamIndex)
{
    if (TeamIndex == 0) ScoreTeamA++;
    else                ScoreTeamB++;
}

void ACTF_GameState::AddPlayerToTeam(int32 TeamIndex)
{
    if (TeamIndex == 0) TeamASize++;
    else                TeamBSize++;
}

// --- Getters ---

int32 ACTF_GameState::GetTeamScore(int32 TeamIndex) const
{
    return (TeamIndex == 0) ? ScoreTeamA : ScoreTeamB;
}

int32 ACTF_GameState::GetTeamSize(int32 TeamIndex) const
{
    return (TeamIndex == 0) ? TeamASize : TeamBSize;
}

int32 ACTF_GameState::GetLeadingTeam() const
{
    if (ScoreTeamA >= ScoreTeamB) return 0;
    return 1;
}

// --- RepNotify callbacks ---

void ACTF_GameState::OnRep_MatchState()
{
    // Los clientes reaccionan al cambio de estado
    // Por ahora lo dejamos vacío, lo conectamos con la UI más adelante
    UE_LOG(LogTemp, Log, TEXT("MatchState changed: %d"), (int32)MatchState);
}

void ACTF_GameState::OnRep_RemainingTime()
{
    // Los clientes actualizan el HUD con el nuevo tiempo
    // Lo conectamos con la UI más adelante
    UE_LOG(LogTemp, Log, TEXT("RemainingTime: %.0f"), RemainingTime);
}

void ACTF_GameState::OnRep_WinnerTeam()
{
    // Los clientes muestran pantalla de victoria/derrota
    UE_LOG(LogTemp, Log, TEXT("WinnerTeam: %d"), WinnerTeam);
}