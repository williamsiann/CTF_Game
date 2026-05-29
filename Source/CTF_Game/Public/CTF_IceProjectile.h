#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CTF_IceProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class CTF_GAME_API ACTF_IceProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ACTF_IceProjectile();

protected:
	// La esfera física que choca contra las paredes y jugadores
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	// El motor físico que hace que la bola viaje, caiga y rebote
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// Evento nativo de Unreal que se dispara en el milisegundo que choca contra algo
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
};