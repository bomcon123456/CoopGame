// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASWeapon::Fire()
{
	// Trace the world, from pawn eyes to crosshair location
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		/* This is not suitable for third person shooter (because get the eye viewpoint by using the Base Eye Height!)
		 * But we've already override one function which will be call in this GetActorEyesViewpoint (see Character.h)
		 * Which will change the viewpoint to the camera viewpoint.
		 */
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 1000);
		// Particle "Target" parameter for the TracerEffect. If doesn't hit, take the tracend, if hit take the hit.impactpoint
		FVector TracerEndPoint = TraceEnd;
		/*
		* We ignore the player/ the owner and the gun itself
		* We set trace complex to true => trace every triangle in the mesh we loooking at (more expensive but more effective)
		* We set it to false => look at the simple collision only. hit too easy
		* LineTrace return true when has a blocking hit
		* If statement true => Process damage because hit.
		*/
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();

			/*
			 * Using ApplyPointDamage has more access to change the information about the damage than the ApplyDamage
			 * Such as from what direction, on what point we hit it.
			 * We can apply physics simple from that
			 * @2ndparam is BaseDamage
			 */
			UGameplayStatics::ApplyPointDamage(
				HitActor,
				20.0f,
				ShotDirection,
				Hit,
				MyOwner->GetInstigatorController(),
				this,
				DamageType);
			if (ImpactEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(),true);
			}
			TracerEndPoint = Hit.ImpactPoint;

		}
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);

		/* THIS SECTION BELOW IS CALLED ALL THE TIME EVEN WE DON'T HIT ANYTHING.
		 * Because the character and the gun will change rapidly, so we have to attached the emitter
		 * Or it will spawn at one place which is stupid
		 * Other params is specified already and we use it.
		 * If Particle Effect is not assigned it will not run
		 */
		if (MuzzleEffect)
		{
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
		}

		if (TracerEffect)
		{
			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
			UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
			if (TracerComp)
			{
				TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
			}
		}
	}
}
