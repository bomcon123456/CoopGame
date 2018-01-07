// Fill out your copyright notice in the Description page of Project Settings.

#include "SGrenadeLauncher.h"

void ASGrenadeLauncher::Fire()
{
	AActor* MyOwner = GetOwner();
	if (MyOwner && ProjectileClass)
	{
		/*
		 * Set up the Transform for the projectile.
		 * MuzzleSocketName is from the SWeapon class, editable in BP 
		 * If we use the SocketRotation, we only shoot with the direction the gun aiming at, so change to EyeRotation for aiming.
		 */
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
	//	FRotator MuzzleRotation = MeshComp->GetSocketRotation(MuzzleSocketName);
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		/*
		 * Set up the Spawn Collision Handling Override ( in case the projectile collide sth)
		 */
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, ActorSpawnParams);
	}
}
