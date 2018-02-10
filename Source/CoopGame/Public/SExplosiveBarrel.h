// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class UStaticMeshComponent;
class UCapsuleComponent;
class USHealthComponent;
class URadialForceComponent;
class UParticleSystem;

UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComp;
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	URadialForceComponent* RForceComp;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UParticleSystem* ExplodingVFX;
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UMaterialInterface* ExplodedMat;

	UPROPERTY(ReplicatedUsing = OnRep_Exploded)
	bool bExploded;


	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
			const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
	UFUNCTION()
	void OnRep_Exploded();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
