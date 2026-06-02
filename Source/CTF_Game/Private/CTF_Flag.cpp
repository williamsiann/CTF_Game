#include "CTF_Flag.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h" 
#include "GameFramework/Character.h"
#include "CTF_PlayerState.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"


// --- Constructor ---
ACTF_Flag::ACTF_Flag()
{
    PrimaryActorTick.bCanEverTick = false;
    
    bReplicates = true;
    SetReplicateMovement(true);
    
    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    SetRootComponent(RootComp);

    // Esfera de Interacción y la atachamos al Root
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComp);
    InteractionSphere->InitSphereRadius(150.f); // Radio de detección
    InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Solo Overlap

    // Malla de la Bandera y la atachamos al Root
    FlagMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlagMesh"));
    FlagMesh->SetupAttachment(RootComp);
    FlagMesh->SetCollisionProfileName(TEXT("NoCollision")); // Sin colisión física
}

void ACTF_Flag::BeginPlay()
{
    Super::BeginPlay();

    PosicionInicial = GetActorLocation();
    RotacionInicial = GetActorRotation();
}

void ACTF_Flag::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// --- Reglas de Replicación de Variables ---
void ACTF_Flag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Aquí registramos que CurrentState se va a replicar a todos los clientes
    DOREPLIFETIME(ACTF_Flag, CurrentState);
}

// --- Implementación de la Interfaz ---

bool ACTF_Flag::CanInteract(AActor* Interactor) const
{
    // Solo permitimos que la levanten si NO está ya equipada por alguien
    return CurrentState != EFlagState::Equipped;
}

void ACTF_Flag::OnInteract(AActor* Interactor)
{
    if (!HasAuthority() || !Interactor) return;
    
    // --- FILTRO NUEVO: Bloquear si es el mismo equipo ---
    // Pasamos el Interactor a APawn para poder pedirle su PlayerState
    if (APawn* Pawn = Cast<APawn>(Interactor))
    {
        if (ACTF_PlayerState* PS = Pawn->GetPlayerState<ACTF_PlayerState>())
        {
            // Si el equipo de la bandera es IGUAL al equipo del jugador...
            if (FlagTeam == PS->GetTeam())
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("¡No podés agarrar tu propia bandera! Tenés que defenderla."));
                }
                return; // ¡LA CLAVE! Corta la función acá y no ejecuta el bindeo a la espalda
            }
        }
    }
    // ----------------------------------------------------

    // Si pasó el filtro de arriba, significa que es un enemigo. Todo legal:
    CurrentState = EFlagState::Equipped;
    
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    if (ACharacter* Character = Cast<ACharacter>(Interactor))
    {
        AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("FlagSocket"));
    }

    OnRep_FlagState();
}

void ACTF_Flag::OnRep_FlagState()
{
    switch (CurrentState)
    {
        case EFlagState::Equipped:
            // Aseguramos que los clientes también apaguen la colisión predictiva
            InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
            
        case EFlagState::Dropped:
            // Lógica a futuro para cuando la bandera caiga al suelo
            break;
            
        case EFlagState::Idle_Base:
            // Estado inicial
            break;
    }
}

void ACTF_Flag::OnDropped()
{
    if (CurrentState == EFlagState::Idle_Base) return;

    // 1. Calculamos hacia dónde escupir la bandera ANTES de despegarnos
    FVector OffsetDesplazamiento = FVector::ZeroVector;
    
    // Le preguntamos a Unreal a quién estamos pegados (El personaje)
    if (AActor* Portador = GetAttachParentActor())
    {
        // La tiramos 100 unidades (1 metro) hacia atrás y 50 hacia la derecha del portador
        OffsetDesplazamiento = (Portador->GetActorForwardVector() * -100.f) + (Portador->GetActorRightVector() * 50.f);
    }

    // 2. Nos despegamos de la espalda del jugador
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // 3. Aplicamos el desplazamiento y la enderezamos al mismo tiempo
    FVector NuevaUbicacion = GetActorLocation() + OffsetDesplazamiento;
    FRotator NuevaRotacion = FRotator(0.f, GetActorRotation().Yaw, 0.f); // Recta, pero manteniendo hacia dónde miraba
    
    SetActorLocationAndRotation(NuevaUbicacion, NuevaRotacion);

    // 4. Reactivamos el Actor por las dudas
    SetActorEnableCollision(true);

    // 5. Le devolvemos la vida a la esfera de colisión
    if (InteractionSphere)
    {
        InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); 
    }

    // 6. Actualizamos el estado para que todos sepan que está en el piso
    CurrentState = EFlagState::Dropped;
    OnRep_FlagState(); 
}

void ACTF_Flag::DevolverABase()
{
    if (HasAuthority())
    {
        // Teletransportamos la bandera a las coordenadas originales
        SetActorLocationAndRotation(PosicionInicial, RotacionInicial);
        
        // Mensaje global (opcional) para avisar que la bandera volvió
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("¡Bandera recuperada y devuelta a la base!"));
    }
} 