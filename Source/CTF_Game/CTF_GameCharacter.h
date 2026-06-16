// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CTF_Interactable.h"
#include "CTF_Damageable.h"

#include "Blueprint/UserWidget.h"
#include "CTF_GameCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

class ACTF_Flag; 


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(abstract)
class ACTF_GameCharacter : public ACharacter, public ICTF_Interactable, public ICTF_Damageable

{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Archivo maestro de controles (Mapping Context) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputAction* AimAction;
	
	FVector DefaultCameraOffset;
	
	FVector AimingCameraOffset;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputAction* FireAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MiraWidgetClass;
	
	UPROPERTY()
	
	UUserWidget* MiraWidgetInstance;

	bool bPickupCooldownActive;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PantallaCongeladaClass;
	
	UPROPERTY()
	UUserWidget* PantallaCongeladaInstance;
	
	float TargetFreezeOpacity;
	float CurrentFreezeOpacity;
	
	FTimerHandle TimerHandle_PickupCooldown;
	
	void FinalizarCooldownPickup();
	void Tick(float DeltaTime);

	float TotalFreezeTime;
	
public:

	/** Constructor */
	ACTF_GameCharacter();

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);



public:

	// Función que el Widget va a usar para saber cómo dibujar la barra (de 0.0 a 1.0)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "CTF|Congelamiento")
	float GetFreezeProgress() const;

	// Evento nativo de Unreal que se dispara al chocar/pisar otro actor
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void OnDropped() override;

	// 1. Agregá este Getter para que la base pueda preguntar si tiene la bandera
	AActor* GetCarriedItem() const { return CarriedItem; }
	
	// Server RPC para soltar lo que tenga en la mano
	UFUNCTION(Server, Reliable)
	void Server_DropItem();

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	// --- ICTF_Interactable ---
	virtual void OnInteract(AActor* Interactor) override;
	virtual bool CanInteract(AActor* Interactor) const override;

	// --- ICTF_Damageable ---
	virtual void OnDamaged(float DamageAmount, AActor* DamageCauser) override;
	virtual void OnDeath(AActor* Killer) override;
	virtual void OnRespawn() override;

	// La bandera se asign al jugador
	void EquipItem(AActor* ItemToEquip);
	
	UFUNCTION()
	void OnRep_Team();
	
	UPROPERTY(ReplicatedUsing = OnRep_Team, EditAnywhere, BlueprintReadWrite, Category = "CTF Skins")
	int32 TeamID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CTF Skins")
	USkeletalMesh* SkinEquipo0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CTF Skins")
	USkeletalMesh* SkinEquipo1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CTF Combat")
	TSubclassOf<class ACTF_Weapon> DefaultWeaponClass;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CTF Combat")
	class ACTF_Weapon* EquippedWeapon;
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnFire();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartAiming();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopAiming();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Combat")
	bool bIsAiming;
    UFUNCTION(Server, Reliable)
    void Server_SetAiming(bool bIsAimingState);
	
	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector& TargetLocation);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Congelar(float Duracion);

protected:

	// Si está vivo
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead;

	// Salud actual
	UPROPERTY(Replicated)
	float Health;
	
	// Salud máxima
	UPROPERTY(EditDefaultsOnly, Category = "CTF")
	float MaxHealth;

	// Timer para el respawn
	FTimerHandle RespawnTimerHandle;
	
	UPROPERTY(EditDefaultsOnly, Category = "CTF")
	float RespawnTime;

	UFUNCTION()
	void OnRep_IsDead();
	
	UPROPERTY(Replicated)
	AActor* CarriedItem;
	
	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* Target);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnRespawn();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	FTimerHandle TimerHandle_Descongelar;
	
	void Descongelar();
	void OnCongelado(float Duracion);
};

