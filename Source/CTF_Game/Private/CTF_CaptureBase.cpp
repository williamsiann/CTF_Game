#include "CTF_CaptureBase.h"
#include "Components/BoxComponent.h"
#include "CTF_GameCharacter.h"
#include "CTF_GameMode.h"
#include "CTF_PlayerState.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "CTF_Flag.h"

ACTF_CaptureBase::ACTF_CaptureBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// zona de captura
	CaptureZone = CreateDefaultSubobject<UBoxComponent>(TEXT("CaptureZone"));
	RootComponent = CaptureZone;
	CaptureZone->SetBoxExtent(FVector(200.f, 200.f, 100.f)); 
	CaptureZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    
	TeamIndex = 0; // Por defecto, Equipo A
}

void ACTF_CaptureBase::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);
    
    if (!HasAuthority()) return;

    if (ACTF_GameCharacter* Player = Cast<ACTF_GameCharacter>(OtherActor))
    {
        AActor* Item = Player->GetCarriedItem();
        
        if (Item != nullptr)
        {
            // --- ACÁ ENTRÁS CON LA BANDERA ---
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Purple, TEXT("¡El jugador entró con la bandera!"));
            }

            ACTF_PlayerState* PS = Player->GetPlayerState<ACTF_PlayerState>();
            if (PS)
            {
                // Verificación de equipo para anotar
                if (PS->GetTeam() == TeamIndex)
                {
                    if (ACTF_Flag* FlagItem = Cast<ACTF_Flag>(Item))
                    {
                        if (FlagItem->FlagTeam != PS->GetTeam())
                        {
                            
                            if (ACTF_GameMode* GM = GetWorld()->GetAuthGameMode<ACTF_GameMode>())
                            {
                                GM->OnFlagCaptured(TeamIndex);
                            }

                            Player->Server_DropItem();
                            FlagItem->DevolverABase();
                        }
                        else
                        {
                            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("¡No podés anotar con tu propia bandera! Andá a robar la del enemigo."));
                        }
                    }
                }
                else
                {
                    // --- BASE ENEMIGA ---
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("¡Alerta! Esta base es del equipo enemigo"));
                    }
                }
            }
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("El jugador entró sin la bandera (No hay punto)"));
            }
        }
    }
}
