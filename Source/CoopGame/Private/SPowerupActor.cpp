// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.0f;
	TotalNrOfTicks = 0;
	bIsPowerupActive = false;

	SetReplicates(true);
}


void ASPowerupActor::OnTickPowerup()
{
	UE_LOG(LogTemp, Warning, TEXT("Ontick"))

	TicksProcessed++;

	OnPowerupTicked();
	UE_LOG(LogTemp, Warning, TEXT("OntickBP"))

	if (TicksProcessed >=  TotalNrOfTicks)
	{
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();	

		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}

void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChange(bIsPowerupActive);
}

// Only called by the server.
void ASPowerupActor::ActivatePowerup()
{
	OnActivated();

	bIsPowerupActive = true;
	// When this variable changes, it's already called this func for all the clients, but not the server
	// So we have to manualy call this so that the server can call this func to.
	OnRep_PowerupActive();
	UE_LOG(LogTemp, Warning, TEXT("Active."))

	if (PowerupInterval > 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Active.1"))

		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Active.2"))

		OnTickPowerup();
	}
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}