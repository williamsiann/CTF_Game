// Copyright Epic Games, Inc. All Rights Reserved.

#include "CTF_GameCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "CTF_Game.h"
#include "CTF_Flag.h"
#include "CTF_PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"

ACTF_GameCharacter::ACTF_GameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ACTF_GameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACTF_GameCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACTF_GameCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACTF_GameCharacter::Look);
	}
	else
	{
		UE_LOG(LogCTF_Game, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACTF_GameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ACTF_GameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ACTF_GameCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ACTF_GameCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ACTF_GameCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ACTF_GameCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ACTF_GameCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACTF_GameCharacter, CarriedItem);
	DOREPLIFETIME(ACTF_GameCharacter, bIsDead);
	DOREPLIFETIME(ACTF_GameCharacter, Health);
	DOREPLIFETIME(ACTF_GameCharacter, TeamID);
}

// --- BeginPlay ---
void ACTF_GameCharacter::BeginPlay()
{
	Super::BeginPlay();
	MaxHealth   = 100.f; 
	RespawnTime = 5.f;
	Health      = MaxHealth; 
	bIsDead     = false;

	// Esto lee el equipo del servidor automáticamente, no tenés que configurar nada en el editor
	if (ACTF_PlayerState* PS = GetPlayerState<ACTF_PlayerState>())
	{
		TeamID = PS->GetTeam(); 
	}

	if (TeamID == 0 && SkinEquipo0)
	{
		GetMesh()->SetSkeletalMeshAsset(SkinEquipo0);
	}
	else if (TeamID == 1 && SkinEquipo1)
	{
		GetMesh()->SetSkeletalMeshAsset(SkinEquipo1);
	}
}

// --- ICTF_Interactable ---
void ACTF_GameCharacter::OnInteract(AActor* Interactor)
{
    // El Character no es interactuable por otros
}

bool ACTF_GameCharacter::CanInteract(AActor* Interactor) const
{
    return false;
}

// --- ICTF_Damageable ---
void ACTF_GameCharacter::OnDamaged(float DamageAmount, AActor* DamageCauser)
{
    if (!HasAuthority() || bIsDead) return;

    Health -= DamageAmount;
    if (Health <= 0.f)
    {
        OnDeath(DamageCauser);
    }
}

void ACTF_GameCharacter::OnDeath(AActor* Killer)
{
    if (!HasAuthority()) return;

    bIsDead = true;

    // Soltamos la bandera si la teníamos
    if (CarriedItem)
    {
			Server_DropItem();
	}

    Multicast_OnDeath();

    // Timer para respawn
    GetWorldTimerManager().SetTimer(
        RespawnTimerHandle,
        this,
        &ACTF_GameCharacter::OnRespawn,
        RespawnTime,
        false
    );
}

void ACTF_GameCharacter::OnRespawn()
{
    if (!HasAuthority()) return;

    bIsDead = false;
    Health  = MaxHealth;

    Multicast_OnRespawn();
}

// --- Flag ---
void ACTF_GameCharacter::EquipItem(AActor* ItemToEquip)
{
	if (!HasAuthority() || !ItemToEquip) return;
	CarriedItem = ItemToEquip;
    
	// Mantenemos tu lógica del PlayerState intacta
	ACTF_PlayerState* PS = GetPlayerState<ACTF_PlayerState>();
	if (PS) PS->SetHasFlag(true);
}

// --- Server RPCs ---
void ACTF_GameCharacter::Server_Interact_Implementation(AActor* Target)
{
	if (!Target) return;
	
	if (ICTF_Interactable* InteractableTarget = Cast<ICTF_Interactable>(Target))
	{
		InteractableTarget->OnInteract(this);
	}
}

void ACTF_GameCharacter::Server_DropItem_Implementation()
{
	if (!HasAuthority() || !CarriedItem) return;
	
	if (ICTF_Interactable* InteractableItem = Cast<ICTF_Interactable>(CarriedItem))
	{
		InteractableItem->OnDropped();
	}
	
	CarriedItem = nullptr; 

	// Actualiza el PlayerState
	ACTF_PlayerState* PS = GetPlayerState<ACTF_PlayerState>();
	if (PS) PS->SetHasFlag(false);
    
	UE_LOG(LogTemp, Log, TEXT("Item soltado correctamente"));
}

// --- Multicast RPCs ---
void ACTF_GameCharacter::Multicast_OnDeath_Implementation()
{
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GetMesh()->SetVisibility(false);
}

void ACTF_GameCharacter::Multicast_OnRespawn_Implementation()
{
    // Reactivamos todo en todos los clientes
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    //GetMesh()->SetVisibility(true);
}

// --- RepNotify ---
void ACTF_GameCharacter::OnRep_IsDead()
{
    UE_LOG(LogTemp, Log, TEXT("IsDead changed: %s"), bIsDead ? TEXT("Si") : TEXT("No"));
}

void ACTF_GameCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// 1. Evitamos chocar contra nosotros mismos o procesar en los clientes
	if (!HasAuthority() || !OtherActor || OtherActor == this) return;
    
	// 2. Si ya llevamos la bandera, no levantamos otra (Las manos están llenas)
	if (CarriedItem != nullptr) return;

	// 3. Magia pura de interfaces: ¿Lo que pisamos es interactuable?
	if (ICTF_Interactable* InteractableItem = Cast<ICTF_Interactable>(OtherActor))
	{
		// 4. ¿El objeto permite que lo agarren ahora mismo?
		if (InteractableItem->CanInteract(this))
		{
			// --- 5. EL NUEVO FILTRO DE EQUIPO (Igual al de la base) ---
			if (ACTF_Flag* FlagItem = Cast<ACTF_Flag>(OtherActor))
			{
				ACTF_PlayerState* PS = GetPlayerState<ACTF_PlayerState>();
				if (PS)
				{
					// Si la bandera es nuestra, el personaje la ignora y frena acá.
					// ¡Nunca se ejecuta el EquipItem!
					if (FlagItem->FlagTeam == PS->GetTeam())
					{
						// Opcional: Podés poner el cartel naranja acá en vez de en la bandera
						if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("¡Es tu bandera! Defendela."));
						return; 
					}
				}
			}
			// -----------------------------------------------------------

			// 6. Si pasamos todos los filtros (es bandera enemiga), la agarramos
			InteractableItem->OnInteract(this);

			// 7. AHORA SÍ, el personaje se guarda la referencia de forma segura
			EquipItem(OtherActor);
		}
	}
}

void ACTF_GameCharacter::OnDropped()
{
	// Si alguien me choca y llevo algo, lo suelto
	if (HasAuthority() && CarriedItem != nullptr)
	{
		Server_DropItem();
	}
}

void ACTF_GameCharacter::OnRep_Team()
{
	// Esta función cambia la malla del personaje según el equipo
	if (TeamID == 0 && SkinEquipo0)
	{
		GetMesh()->SetSkeletalMeshAsset(SkinEquipo0);
	}
	else if (TeamID == 1 && SkinEquipo1)
	{
		GetMesh()->SetSkeletalMeshAsset(SkinEquipo1);
	}
}