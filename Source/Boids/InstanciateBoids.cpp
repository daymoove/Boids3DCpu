#include "InstanciateBoids.h"

#include "VectorTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"




// Sets default values
AInstanciateBoids::AInstanciateBoids()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedMesh"));
	SetRootComponent(mesh);
	mesh->SetMobility(EComponentMobility::Static);
	mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

// void AInstanciateBoids::OnConstruction(const FTransform& Transform)
// {
// 	Super::OnConstruction(Transform);
//
// 	
// }


// Called when the game starts or when spawned
void AInstanciateBoids::BeginPlay()
{
	Super::BeginPlay();
	if (mesh->GetInstanceCount() == 0)
	{
		transforms.Empty(1000);
		for (int i = 0; i < 1000; ++i)
		{
			transforms.Add(FTransform(FVector(FMath::RandRange(0,2000), FMath::RandRange(0,2000), FMath::RandRange(0,2000))));
			transforms[i].SetRotation(FQuat(FRotator(FMath::RandRange(-90,90),FMath::RandRange(-90,90),0)));
		}
		mesh->AddInstances(transforms,false);
	}
}

// Called every frame
void AInstanciateBoids::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AlignBoids();
	for (int i = 0; i < 1000; ++i)
	{
		transforms[i].SetLocation(transforms[i].GetLocation() += alignment * 50 * DeltaTime);
		mesh->UpdateInstanceTransform(i, transforms[i], false, true);
	}
}

void AInstanciateBoids::AlignBoids()
{
	FVector sum = FVector(0, 0, 0);
	for (FTransform t : transforms)
	{
		sum += t.GetRotation().GetUpVector();
	}
	alignment = sum.GetSafeNormal();
}

