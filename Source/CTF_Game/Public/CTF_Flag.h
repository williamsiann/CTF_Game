#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CTF_Interactable.h" // Incluimos la interfaz
#include "CTF_Flag.generated.h"

// Enum para manejar el estado de la bandera y replicarlo a los clientes
UENUM(BlueprintType)
enum class EFlagState : uint8
{
	Idle_Base       UMETA(DisplayName = "En Base"),
	Dropped         UMETA(DisplayName = "Tirada en el Suelo"),
	Equipped        UMETA(DisplayName = "Equipada")
};

UCLASS()
class CTF_GAME_API ACTF_Flag : public AActor, public ICTF_Interactable
{
	GENERATED_BODY()
    
public:	
	ACTF_Flag();
	FVector PosicionInicial;
	FRotator RotacionInicial;

protected:
	virtual void BeginPlay() override;
    
	// Función nativa obligatoria para registrar las variables que se van a replicar en red
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	virtual void Tick(float DeltaTime) override;

	// --- Implementación de la Interfaz ICTF_Interactable ---
	virtual void OnInteract(AActor* Interactor) override;
	
	virtual bool CanInteract(AActor* Interactor) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CTF Rules")
	int32 FlagTeam;

protected:
	// --- Componentes ---

	// El ancla principal del actor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootComp;

	// El volumen invisible que detectará si el jugador está cerca
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* FlagMesh;

	// --- Variables Replicadas ---
    
	// El estado actual de la bandera. Usamos OnRep_FlagState para actualizar visuales en los clientes
	UPROPERTY(ReplicatedUsing = OnRep_FlagState, BlueprintReadOnly, Category = "Flag State")
	EFlagState CurrentState;

	UFUNCTION()
	void OnRep_FlagState();
	void OnDropped();
};