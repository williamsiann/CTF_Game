#include "CTF_GameMode.h"
#include "CTF_GameState.h"
#include "CTF_PlayerState.h"
#include "CTF_PlayerController.h"
#include "Engine/World.h"         
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

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

    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    int32 TeamA = GS->GetTeamSize(0);
    int32 TeamB = GS->GetTeamSize(1);

    int32 AssignedTeam = (TeamA <= TeamB) ? 0 : 1;
    
    // IMPORTANTE: Descomentá esto para que el PlayerState guarde el equipo
    PS->SetTeam(AssignedTeam); 
    
    GS->AddPlayerToTeam(AssignedTeam);

    // --- CARTEL DE BUG PARA LOG Y PANTALLA ---
    FString LoginMsg = FString::Printf(TEXT("¡Jugador %s asignado al EQUIPO %d!"), *NewPlayer->GetName(), AssignedTeam);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LoginMsg);
    
    if (GEngine)
    {
        // Lo ponemos en color celeste (Cyan) por 10 segundos para que te dé tiempo a verlo
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan, LoginMsg);
    }
}

void ACTF_GameMode::StartMatch()
{
    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    GS->SetMatchState(EMatchState::InProgress);
    GS->SetRemainingTime(MatchTime);

    // Arranca el timer que descuenta tiempo
    GetWorldTimerManager().SetTimer(
        MatchTimerHandle,
        this,
        &ACTF_GameMode::OnMatchTimerTick,
        1.f,   // cada 1 segundo
        true   // loop
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
        // Se acabó el tiempo, gana quien tenga más puntos
        int32 Winner = GS->GetLeadingTeam();
        EndMatch(Winner);
    }
}

void ACTF_GameMode::OnFlagCaptured(int32 TeamIndex)
{
    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;
    
    GS->AddScore(TeamIndex);

    // --- CARTEL PARA VER EL MARCADOR EN PANTALLA ---
    if (GEngine)
    {
        FString ScoreMsg = FString::Printf(TEXT("MARCADOR GLOBAL ➔ Equipo 0: %d | Equipo 1: %d"), GS->GetTeamScore(0), GS->GetTeamScore(1));
        GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Emerald, ScoreMsg);
    }

    // Chequeamos si alguien llegó al límite para ganar
    CheckVictoryCondition(TeamIndex);
}

void ACTF_GameMode::CheckVictoryCondition(int32 TeamIndex)
{
    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    // Preguntamos si el equipo que acaba de anotar llegó al límite
    // (Nota: uso ScoreToWin que es la variable que ya tenías en tu GameMode)
    if (GS->GetTeamScore(TeamIndex) >= ScoreToWin)
    {
        // ¡LLAMAMOS A ENDMATCH! Esta es la conexión vital
        EndMatch(TeamIndex);
    }
}

void ACTF_GameMode::EndMatch(int32 WinnerTeam)
{
    // 1. Frena el reloj 
    GetWorldTimerManager().ClearTimer(MatchTimerHandle);

    ACTF_GameState* GS = GetGameState<ACTF_GameState>();
    if (!GS) return;

    // 2. Cambia el estado del juego 
    GS->SetMatchState(EMatchState::GameOver);
    GS->SetWinnerTeam(WinnerTeam);

    // --- 3. ACÁ VAN LOS EFECTOS ÉPICOS ---
    FString WinMsg = FString::Printf(TEXT("¡¡EL EQUIPO %d HA GANADO LA PARTIDA!!"), WinnerTeam);
    
    if (GEngine)
    {
        // Cartel gigante rojo
        GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, WinMsg, true, FVector2D(3.0f, 3.0f));
    }

    // Cámara lenta al 20%
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
    // -----------------------------------------

    // 4. Notifica a todos los PlayerControllers 
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        ACTF_PlayerController* PC = Cast<ACTF_PlayerController>(It->Get());
        if (PC)
        {
            PC->OnMatchEnded(WinnerTeam);
        }
    }
}