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

    // Esfera de Interacción y la atacha al Root
    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(RootComp);
    InteractionSphere->InitSphereRadius(150.f); // Radio de detección
    InteractionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic")); // Solo Overlap

    // Malla de la Bandera y la atacha al Root
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
    
    DOREPLIFETIME(ACTF_Flag, CurrentState);
}

// --- Implementación de la Interfaz ---

bool ACTF_Flag::CanInteract(AActor* Interactor) const
{
    return CurrentState != EFlagState::Equipped;
}

void ACTF_Flag::OnInteract(AActor* Interactor)
{
    if (!HasAuthority() || !Interactor) return;
    
    if (APawn* Pawn = Cast<APawn>(Interactor))
    {
        if (ACTF_PlayerState* PS = Pawn->GetPlayerState<ACTF_PlayerState>())
        {
            if (FlagTeam == PS->GetTeam())
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("¡No podés agarrar tu propia bandera! Tenés que defenderla."));
                }
                return; 
            }
        }
    }
    // ----------------------------------------------------
    
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
            InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            break;
            
        case EFlagState::Dropped:
            break;
            
        case EFlagState::Idle_Base:
            break;
    }
}

void ACTF_Flag::OnDropped()
{
    if (CurrentState == EFlagState::Idle_Base) return;
    
    FVector OffsetDesplazamiento = FVector::ZeroVector;
    
    if (AActor* Portador = GetAttachParentActor())
    {
        OffsetDesplazamiento = (Portador->GetActorForwardVector() * -100.f) + (Portador->GetActorRightVector() * 50.f);
    }
    
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    
    FVector NuevaUbicacion = GetActorLocation() + OffsetDesplazamiento;
    FRotator NuevaRotacion = FRotator(0.f, GetActorRotation().Yaw, 0.f); 
    
    SetActorLocationAndRotation(NuevaUbicacion, NuevaRotacion);
    
    SetActorEnableCollision(true);
    
    if (InteractionSphere)
    {
        InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); 
    }
    
    CurrentState = EFlagState::Dropped;
    OnRep_FlagState(); 
}

void ACTF_Flag::DevolverABase()
{
    if (HasAuthority())
    {
        DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        
        SetActorLocationAndRotation(PosicionInicial, RotacionInicial);
        
        SetActorEnableCollision(true);
        if (InteractionSphere)
        {
            InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); 
        }
        
        CurrentState = EFlagState::Idle_Base;
        OnRep_FlagState();
        
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("¡Bandera recuperada y devuelta a la base!"));
    }
}