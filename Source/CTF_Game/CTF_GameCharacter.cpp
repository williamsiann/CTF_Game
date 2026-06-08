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
#include "CTF_Weapon.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

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

	bPickupCooldownActive = false;

	
	PrimaryActorTick.bCanEverTick = true;

	// Inicializo el hielo apagado
	CurrentFreezeOpacity = 0.0f;
	TargetFreezeOpacity = 0.0f;
	TotalFreezeTime = 0.0f;
}


void ACTF_GameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACTF_GameCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ACTF_GameCharacter::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACTF_GameCharacter::Look);
		
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ACTF_GameCharacter::OnFire);
		}
		
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ACTF_GameCharacter::StartAiming);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ACTF_GameCharacter::StopAiming);
		}
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
	DOREPLIFETIME(ACTF_GameCharacter, EquippedWeapon);
	DOREPLIFETIME(ACTF_GameCharacter, bIsAiming);
}

// --- BeginPlay ---
void ACTF_GameCharacter::BeginPlay()
{
	Super::BeginPlay();
	MaxHealth   = 100.f; 
	RespawnTime = 5.f;
	Health      = MaxHealth; 
	bIsDead     = false;
	
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
	if (HasAuthority() && DefaultWeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		EquippedWeapon = GetWorld()->SpawnActor<ACTF_Weapon>(DefaultWeaponClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (EquippedWeapon)
		{
			EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Hand_R_Socket"));
		}
	}
	if (CameraBoom)
	{
		DefaultCameraOffset = CameraBoom->SocketOffset; 
  
		AimingCameraOffset = FVector(0.f, 50.f, 50.f); 
	}
	if (IsLocallyControlled() && MiraWidgetClass)
	{
		MiraWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), MiraWidgetClass);
		if (MiraWidgetInstance)
		{
			MiraWidgetInstance->AddToViewport();
			// La ocultamos de entrada porque no estamos apuntando
			MiraWidgetInstance->SetVisibility(ESlateVisibility::Hidden); 
		}
	}
	// Creamos la pantalla de hielo solo para el jugador local
	if (IsLocallyControlled() && PantallaCongeladaClass)
	{
		PantallaCongeladaInstance = CreateWidget<UUserWidget>(GetWorld(), PantallaCongeladaClass);
		if (PantallaCongeladaInstance)
		{
			PantallaCongeladaInstance->AddToViewport();
			// Usamos HitTestInvisible para que el hielo visual NO bloquee los clics del mouse
			PantallaCongeladaInstance->SetVisibility(ESlateVisibility::HitTestInvisible);
			PantallaCongeladaInstance->SetRenderOpacity(0.0f); // Transparente total
		}
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

	bPickupCooldownActive = true;

	GetWorldTimerManager().SetTimer(
		TimerHandle_PickupCooldown,
		this,
		&ACTF_GameCharacter::FinalizarCooldownPickup,
		5.0f, // Los 5 segundos de espera
		false
	);
    
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
	
	if (!HasAuthority() || !OtherActor || OtherActor == this) return;

	if (bPickupCooldownActive) return;
	
	if (CarriedItem != nullptr) return;
	
	if (ICTF_Interactable* InteractableItem = Cast<ICTF_Interactable>(OtherActor))
	{
		
		if (InteractableItem->CanInteract(this))
		{
			
			if (ACTF_Flag* FlagItem = Cast<ACTF_Flag>(OtherActor))
			{
				ACTF_PlayerState* PS = GetPlayerState<ACTF_PlayerState>();
				if (PS)
				{
				
					if (FlagItem->FlagTeam == PS->GetTeam())
					{
						FlagItem->DevolverABase();
						return; 
					}
				}
			}
			
			InteractableItem->OnInteract(this);

			EquipItem(OtherActor);
		}
	}
}

void ACTF_GameCharacter::OnDropped()
{
	if (HasAuthority() && CarriedItem != nullptr)
	{
		Server_DropItem();
	}
}

void ACTF_GameCharacter::OnRep_Team()
{
	if (TeamID == 0 && SkinEquipo0)
	{
		GetMesh()->SetSkeletalMeshAsset(SkinEquipo0);
	}
	else if (TeamID == 1 && SkinEquipo1)
	{
		GetMesh()->SetSkeletalMeshAsset(SkinEquipo1);
	}
}

void ACTF_GameCharacter::OnFire()
{

	if (!EquippedWeapon)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ERROR: Clic detectado, pero no hay arma equipada"));
		return;
	}
	//  lógica original de trazado y disparo
	if (FollowCamera)
	{
		FVector CameraLoc = FollowCamera->GetComponentLocation();
		FVector CameraForward = FollowCamera->GetForwardVector();
		FVector TraceEnd = CameraLoc + (CameraForward * 10000.f);

		FHitResult HitResult;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CameraLoc, TraceEnd, ECC_Visibility, TraceParams);
		FVector TargetLocation = bHit ? HitResult.ImpactPoint : TraceEnd;

		// Llamamos al servidor
		Server_Fire(TargetLocation);
	}
}

void ACTF_GameCharacter::Server_Fire_Implementation(const FVector& TargetLocation)
{
	if (!EquippedWeapon) return;
	
	FVector MuzzleLocation = EquippedWeapon->GetRootComponent()->GetSocketLocation(TEXT("MuzzleSocket"));
	
	FRotator LaunchRotation = (TargetLocation - MuzzleLocation).Rotation();

	EquippedWeapon->Fire(MuzzleLocation, LaunchRotation, this);
}

void ACTF_GameCharacter::StartAiming()
{
	bIsAiming = true; 
	Server_SetAiming(true); 

	if (CameraBoom)
	{
		CameraBoom->SocketOffset = AimingCameraOffset;
	}
	if (IsLocallyControlled() && MiraWidgetInstance)
	{
		MiraWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	}
}

void ACTF_GameCharacter::StopAiming()
{
	bIsAiming = false; 
	Server_SetAiming(false); 

	if (CameraBoom)
	{
		CameraBoom->SocketOffset = DefaultCameraOffset;
	}
	
	if (IsLocallyControlled() && MiraWidgetInstance)
	{
		MiraWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ACTF_GameCharacter::Server_SetAiming_Implementation(bool bIsAimingState)
{
	bIsAiming = bIsAimingState;
}

void ACTF_GameCharacter::Multicast_Congelar_Implementation(float Duracion)
{
	GetCharacterMovement()->DisableMovement();
	
	if (GetMesh())
	{
		GetMesh()->bPauseAnims = true;
	}
	
	GetWorldTimerManager().SetTimer(
		TimerHandle_Descongelar,
		this,
		&ACTF_GameCharacter::Descongelar,
		Duracion,
		false
	);
	
	GetWorldTimerManager().SetTimer(
		TimerHandle_Descongelar,
		this,
		&ACTF_GameCharacter::Descongelar,
		Duracion,
		false
	);

	TotalFreezeTime = Duracion;
	
	if (IsLocallyControlled())
	{
		TargetFreezeOpacity = 1.0f; 
	}
}

void ACTF_GameCharacter::Descongelar()
{
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	if (GetMesh())
	{
		GetMesh()->bPauseAnims = false;
	}

	if (IsLocallyControlled())
	{
		TargetFreezeOpacity = 0.0f; 
	}
}

void ACTF_GameCharacter::OnCongelado(float Duracion)
{
	if (HasAuthority())
	{
		if (CarriedItem != nullptr)
		{
			Server_DropItem();
		}
		
		Multicast_Congelar(Duracion);
	}
}

void ACTF_GameCharacter::FinalizarCooldownPickup()
{
	bPickupCooldownActive = false;
    
	if (GEngine) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("¡Ya podés volver a agarrar la bandera!"));
	}
}
void ACTF_GameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PantallaCongeladaInstance && !FMath::IsNearlyEqual(CurrentFreezeOpacity, TargetFreezeOpacity))
	{
		CurrentFreezeOpacity = FMath::FInterpTo(CurrentFreezeOpacity, TargetFreezeOpacity, DeltaTime, 5.0f);
        
		PantallaCongeladaInstance->SetRenderOpacity(CurrentFreezeOpacity);
	}
}

float ACTF_GameCharacter::GetFreezeProgress() const
{
	if (TotalFreezeTime <= 0.0f) 
	{
		return 0.0f;
	}
	float TiempoRestante = GetWorldTimerManager().GetTimerRemaining(TimerHandle_Descongelar);
	if (TiempoRestante <= 0.0f) 
	{
		return 0.0f;
	}
	return TiempoRestante / TotalFreezeTime;
}