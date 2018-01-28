// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

/*			CREATING CUSTOM EVENT 
 * Custom event like (OnHitEvent, OnBeginOverlapEvent,...) which when we show in BP (using BlueprintAssignable)
 * We can implement it in the BP_class.
 * On Health Change Event.
 * Allow us to access to react to Health Changing event
 * If health < 0; we can play death animation
 * If health == sth, we can play sound grunting , blah blah.
 * @1st-param : The name of event type.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Health will call this function when replicated => Client can have it too.
	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;
	
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


	/*
	 * !!!! Trick:
	 * When you replicateUsing, if you let it has 1 parameter, that parameter will hold the old value of that variable
	 * In this case, Health is replicated => the Health before replicating will be loaded in to "OldHealth"
	 * Use to calculate the damage caused to the owner of this comp.
	 */
	UFUNCTION()
	void OnRep_Health(float OldHealth);
public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

};
