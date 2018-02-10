// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::HandleTakeDamage);

	RForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RForceComp"));
	RForceComp->SetupAttachment(RootComponent);
	RForceComp->ImpulseStrength = 1000.0f;
	RForceComp->Radius = 260.0f;
	//RForceComp->ForceStrength = 500000.f;
	RForceComp->bImpulseVelChange = true;
	RForceComp->bAutoActivate = false;		// Set true will run Tick function.
	RForceComp->bIgnoreOwningActor = true;	// Ignore self.

	SetReplicates(true);
	SetReplicateMovement(true);

}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASExplosiveBarrel::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded) { return; }
	if(Health <= 0.0f)
	{
		bExploded = true;
		OnRep_Exploded();
			// Add Force to nearby
			RForceComp->FireImpulse();

			// Launch up
			FVector FlyUp = FVector(800.0f, 1500.f, 8000.0f);
			MeshComp->AddForce(FlyUp, NAME_None, true);

	}
}

void ASExplosiveBarrel::OnRep_Exploded()
{
	// Spawn VFX
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodingVFX, GetActorLocation());
	
	// Change Material 
	if (!ExplodedMat)
	{
		UE_LOG(LogTemp, Warning, TEXT("Material not found"))
	}
	MeshComp->SetMaterial(0, ExplodedMat);

}

// Called every frame
void ASExplosiveBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}
