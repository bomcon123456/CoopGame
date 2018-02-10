// Fill out your copyright notice in the Description page of Project Settings.

#include "STracketBot.h"
#include "Components/StaticMeshComponent.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ASTracketBot::ASTracketBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// MeshComp
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));

	// So the mesh will not affected in the Nav Modifier Volume.
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	bUseVelocityChange = true;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;

	// Health Comp.
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTracketBot::HandleTakeDamage);

	SelfDamageInterval = 0.25f;
	ExplosionDamage = 40;
	ExplosionRadius = 200;

	// Sphere Comp.
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ASTracketBot::BeginPlay()
{
	Super::BeginPlay();

	// Find initial next path point on ServerOnly because Nav only exists in Server
	if (Role == ROLE_Authority) 
	{
		NextPathPoint = GetNextPathPoint();
	}

}

FVector ASTracketBot::GetNextPathPoint()
{
	/*		WILL NOT WORK AT MULTIPLAYER.
	 * This function is used to find path to the actor, returning an array of FVector trace from
	 * this actor location and get to the player location
	 * The rest param is used as default.
	 * The first PathPoint is where this actor located, so we need from the second one.
	 */
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);
	UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (NavPath->PathPoints.Num() > 1)
	{
		// Return next point it the path. 
		return NavPath->PathPoints[1];
	}

	// Failed to find path;
	return GetActorLocation();
}

void ASTracketBot::DamageSelf()
{
	// Do 20 damage every time called
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ASTracketBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// Explode on death.

	/*
	 * First param is the index of the material (the first)
	 * So does the second param.
	 * We use dynamic instance so we have the power the change material only in one Actor's instance, not all
	 */
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Warning, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName())

	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

void ASTracketBot::SelfDestruct()
{
	if (bExploded) { return; }
	bExploded = true;
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Apply damage => Server Only, then Health is replicated => the client will receive.
	if(Role == ROLE_Authority)
	{
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius,
			nullptr, IgnoredActors, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Yellow, false, 2.0f, 0, 1.0f);

		// Destroy after 2 sec
		SetLifeSpan(2.0f);
	}
}

// Called every frame
void ASTracketBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Only server do this because we call GetNextPathPoint too.
	if(Role == ROLE_Authority && !bExploded)
	{
		// Size function is the "Sqrt(X*X + Y*Y + Z*Z)" one, which gets the distance.
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)		// If the actor is closer than 100 cm.
		{
			NextPathPoint = GetNextPathPoint();
			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
		}
		else
		{
			// Keep moving towards next path point (target)
			/*
			* From the world:
			* This actor has a vector A(x,y,z) which has root at 0,0,0
			* The Next Path Point is a vector P(a,b,c) which also has root at 0,0,0
			* According to the sum of vector: A + ??? = P ( A and P same root)
			* So the direction(with magnitude) will be ??? = P - A.
			* Then normalize to get unit vector only.
			*/
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			FVector ActorForce = ForceDirection * MovementForce;

			MeshComp->AddForce(ActorForce, NAME_None, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Red, false, 0.0f, 0, 1.0f);
		}
		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Red, false, 0.0f, 1.0f);
	}
}

void ASTracketBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn)
		{
			// Set damage => only server.
			// Every 0.5 sec, this function will by call by this timer
			// Start self destruction sequence when the player enter the sphere comp.
			// Timer is server-manage-required
			if (Role == ROLE_Authority)
			{
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTracketBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			bStartedSelfDestruction = true;

			// Sound => both.
			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}
