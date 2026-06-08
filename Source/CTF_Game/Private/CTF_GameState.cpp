#include "CTF_GameState.h"
#include "Blueprint/UserWidget.h"
#include "CTF_PlayerState.h"
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
    OnRep_MatchState(); 
}

void ACTF_GameState::SetRemainingTime(float NewTime)
{
    RemainingTime = NewTime;
    OnRep_RemainingTime(); 
}

void ACTF_GameState::SetWinnerTeam(int32 TeamIndex)
{
    WinnerTeam = TeamIndex;
    OnRep_WinnerTeam();
}

void ACTF_GameState::AddScore(int32 TeamIndex)
{
    if (TeamIndex == 0)
    {
        ScoreTeamA++;
        UE_LOG(LogTemp, Warning, TEXT(">>> AddScore llamado - TeamA: %d"), ScoreTeamA);
        OnRep_ScoreTeamA();
    }
    else
    {
        ScoreTeamB++;
        UE_LOG(LogTemp, Warning, TEXT(">>> AddScore llamado - TeamB: %d"), ScoreTeamB);
        OnRep_ScoreTeamB();
    }
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
    UE_LOG(LogTemp, Log, TEXT("MatchState changed: %d"), (int32)MatchState);
}

void ACTF_GameState::OnRep_RemainingTime()
{

    UE_LOG(LogTemp, Log, TEXT("RemainingTime: %.0f"), RemainingTime);
}

void ACTF_GameState::OnRep_ScoreTeamA()
{
    // Avisamos a los Widgets que se tienen que redibujar
    OnScoreChangedEvent.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Team A Score: %d"), ScoreTeamA);
}


void ACTF_GameState::OnRep_ScoreTeamB()
{
    // Avisamos a los Widgets que se tienen que redibujar
    OnScoreChangedEvent.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Team B Score: %d"), ScoreTeamB);
}

void ACTF_GameState::OnRep_WinnerTeam()
{
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    if (PC && PC->IsLocalController())
    {

        if (ACTF_PlayerState* PS = PC->GetPlayerState<ACTF_PlayerState>())
        {
            int32 MiEquipo = PS->GetTeam();

            if (MiEquipo == WinnerTeam)
            {
                if (VictoriaWidgetClass)
                {
                    UUserWidget* WinWidget = CreateWidget<UUserWidget>(PC, VictoriaWidgetClass);
                    if (WinWidget) WinWidget->AddToViewport();
                }
            }
            else
            {
                if (DerrotaWidgetClass)
                {
                    UUserWidget* LoseWidget = CreateWidget<UUserWidget>(PC, DerrotaWidgetClass);
                    if (LoseWidget) LoseWidget->AddToViewport();
                }
            }
            
            PC->bShowMouseCursor = true;
            FInputModeUIOnly InputMode;
            PC->SetInputMode(InputMode);
        }
    }
}

