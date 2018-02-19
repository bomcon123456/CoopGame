// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

// Contains information of a single hitscan weapon line-trace.
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:
	// Must use TEnumAsByte to replicate Enum type.
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	// Do some vector packing for us (less size, more info)
	UPROPERTY()
	FVector_NetQuantize TraceTo;
};



UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	/*
	 * "Server": when ever this function call outside the server, it will not be use but it'll ask for the server to do it.
	 * "Reliable": guarantee that it will be run by the server.
	 * "WithValidation": create ServerFire_Validation for sanity check.
	 * To Implementation this function, we must let it be void ServerFire_Implementation(){}
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFiring();

	/** Makes the camera Shake too.*/
	void PlayFireEffect(FVector TraceEnd);
	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	FTimerHandle TimerHandle_TimeBetweenShots;
	float LastFireTime;
	// Derived from RateOfFire.
	float TimeBetweenShots;
	/* RPM - Bullets per minutes fire object*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;
	
	// Replicated using OnRep_HitScanTrace function
	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();
public:

	void StartFire();
	void StopFire();
};
