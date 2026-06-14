#include "CTF_PlayerState.h"
#include "Net/UnrealNetwork.h"

ACTF_PlayerState::ACTF_PlayerState()
{
	TeamIndex     = -1; // sin equipo asignado todavía
	IndividualScore = 0;
	bHasFlag      = false;
}

void ACTF_PlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACTF_PlayerState, TeamIndex);
	DOREPLIFETIME(ACTF_PlayerState, IndividualScore);
	DOREPLIFETIME(ACTF_PlayerState, bHasFlag);
}

// --- Setters ---

void ACTF_PlayerState::SetTeam(int32 NewTeam)
{
	TeamIndex = NewTeam;
	OnRep_TeamIndex(); // llamamos manual en servidor
}

void ACTF_PlayerState::AddScore(int32 Amount)
{
	IndividualScore += Amount;
}

void ACTF_PlayerState::SetHasFlag(bool bNewHasFlag)
{
	bHasFlag = bNewHasFlag;
	OnRep_HasFlag(); // llamamos manual en servidor
}

// --- RepNotify callbacks ---

void ACTF_PlayerState::OnRep_TeamIndex()
{
	UE_LOG(LogTemp, Log, TEXT("Player team assigned: %d"), TeamIndex);
}

void ACTF_PlayerState::OnRep_HasFlag()
{
	// Cliente reacciona a tomar/soltar bandera
	UE_LOG(LogTemp, Log, TEXT("Player has flag: %s"), bHasFlag ? TEXT("Yes") : TEXT("No"));
}