// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

/** Console variable. ( when press "`" )*/
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons."),
	ECVF_Cheat);
/** Console variable. ( when press "`" )*/

// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "BeamEnd";
	BaseDamage = 20.0f;
	RateOfFire = 600.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
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
		* ReturnPhysicalMaterial set to true so it can find the PhysMat.
		*/
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;
		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			float ActualDamage = BaseDamage;
			/*
			 * Using ApplyPointDamage has more access to change the information about the damage than the ApplyDamage
			 * Such as from what direction, on what point we hit it.
			 * We can apply physics simple from that
			 * @2ndparam is BaseDamage
			 */
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(
				HitActor,
				ActualDamage,
				ShotDirection,
				Hit,
				MyOwner->GetInstigatorController(),
				this,
				DamageType);
		
			UParticleSystem* SelectedEffect = nullptr;
			switch (SurfaceType)
			{
			case SURFACE_FLESHDEFAULT:
			case SURFACE_FLESHVULNERABLE:
				SelectedEffect = FleshImpactEffect;
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}

			if (SelectedEffect)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), true);
			}

			TracerEndPoint = Hit.ImpactPoint;

		}
		if(DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		/* THIS SECTION BELOW IS CALLED ALL THE TIME EVEN WE DON'T HIT ANYTHING.
		 * Because the character and the gun will change rapidly, so we have to attached the emitter
		 * Or it will spawn at one place which is stupid
		 * Other params is specified already and we use it.
		 * If Particle Effect is not assigned it will not run
		 */
		PlayFireEffect(TracerEndPoint);

		LastFireTime = GetWorld()->TimeSeconds;
	}
}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	// Every 1s, call Fire().
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots,this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffect(FVector TraceEnd)
{
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
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			/* This function is made already in the PlayerController.h, just need first param, others use default.
			 * Then we go to the UE, create a CameraShake BP to config.
			 */
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}
