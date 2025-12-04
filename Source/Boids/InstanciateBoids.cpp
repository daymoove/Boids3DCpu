#include "InstanciateBoids.h"

#include "VectorTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "DynamicMesh/DynamicMesh3.h"
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
		transforms.Empty(numBoids);
		for (int i = 0; i < numBoids; ++i)
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
	for (int i = 0; i < numBoids; ++i)
	{
		transforms[i].SetLocation(transforms[i].GetLocation() += transforms[i].GetRotation().GetUpVector() * speed * DeltaTime);
		mesh->UpdateInstanceTransform(i, transforms[i], false, true);
	}
}

void AInstanciateBoids::AlignBoids()
{
	FVector sum = FVector::ZeroVector;
	for (FTransform t : transforms)
	{
		sum += t.GetRotation().GetUpVector();
	}
	alignment = sum.GetSafeNormal();
}

void AInstanciateBoids::SeparateBoids()
{
	for (FTransform transform : transforms)
	{
		for (FTransform otherTransform: transforms)
		{
			FVector vectorBetweenAandB = transform.GetLocation() - otherTransform.GetLocation();
			float ratio = 1 - (UE::Geometry::Length(vectorBetweenAandB)/maxDistance);
			FVector otherDirection = vectorBetweenAandB.GetSafeNormal();
			separation.Add((transform.GetRotation().GetUpVector()+otherDirection * ratio).GetSafeNormal());
		}
	}
}

void AInstanciateBoids::CohesionBoids()
{
	FVector centroid = FVector::ZeroVector;
	for (FTransform transform : transforms)
	{
		centroid += transform.GetLocation();
	}
	centroid /= numBoids;

	for (FTransform& t : transforms)
	{
		FVector DirectionToCenter = (centroid - t.GetLocation());
		cohesion = DirectionToCenter.GetSafeNormal();
	}

}

void AInstanciateBoids::SetBoidRotation(FTransform& transform ) const
{
	FVector CurrentUp = transform.GetRotation().GetUpVector();
	if (CurrentUp.Equals(alignment, 0.01f)) return;
	FQuat DeltaRotation = FQuat::FindBetween(CurrentUp, alignment);
	transform.SetRotation(DeltaRotation * transform.GetRotation());
}

