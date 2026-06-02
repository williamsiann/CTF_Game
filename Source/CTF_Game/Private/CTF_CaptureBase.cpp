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

    // Solo el servidor maneja la lógica de puntos y mensajes de debug
    if (!HasAuthority()) return;

    if (ACTF_GameCharacter* Player = Cast<ACTF_GameCharacter>(OtherActor))
    {
        // Guardamos lo que lleva el jugador en una variable
        AActor* Item = Player->GetCarriedItem();

        // --- EL PATOVICA: ¿El jugador tiene un objeto en las manos? ---
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
                // Mensaje opcional para ver los equipos en pantalla
                FString TeamInfo = FString::Printf(TEXT("Tu Equipo: %d | Base Equipo: %d"), PS->GetTeam(), TeamIndex);
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, TeamInfo);

                // Verificación de equipo para anotar
                if (PS->GetTeam() == TeamIndex)
                {
                    // Hacemos el Cast a la bandera acá arriba para poder preguntarle su equipo
                    if (ACTF_Flag* FlagItem = Cast<ACTF_Flag>(Item))
                    {
                        // 4. EL FILTRO NUEVO: ¿La bandera es del equipo RIVAL?
                        if (FlagItem->FlagTeam != PS->GetTeam())
                        {
                            // 5. ¡AHORA SÍ! Todo legal. Sumamos el punto.
                            if (ACTF_GameMode* GM = GetWorld()->GetAuthGameMode<ACTF_GameMode>())
                            {
                                GM->OnFlagCaptured(TeamIndex);
                            }

                            Player->Server_DropItem();
                           //FlagItem->SetActorLocationAndRotation(FlagItem->PosicionInicial, FlagItem->RotacionInicial);
                        }
                        else
                        {
                            // El jugador trajo su propia bandera a su propia base (o la agarró apenas spawneó)
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
            // --- ACÁ ENTRÁS SIN LA BANDERA (Y el servidor te ignora) ---
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("El jugador entró sin la bandera (No hay punto)"));
            }
        }
    }
}
