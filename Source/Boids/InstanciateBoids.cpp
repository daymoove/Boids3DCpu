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


// Called when the game starts or when spawned
void AInstanciateBoids::BeginPlay()
{
	Super::BeginPlay();
	finalDirections.SetNumZeroed(numBoids);
	
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
	if (numBoids == 0) return;

	CalculateGlobalData();
	
	ParallelFor(numBoids, [&](int i)
	{
		FVector separationForce = SeparateBoids(i);
		FVector cohesionForce = (centroid - transforms[i].GetLocation()).GetSafeNormal();
		FVector desiredDir = (alignment * 1.0f) + (separationForce * 2.0f) + (cohesionForce * 1.0f);
		finalDirections[i] = desiredDir.GetSafeNormal();
	});
	for (int i = 0; i < numBoids; ++i)
	{
		FVector currentUp = transforms[i].GetRotation().GetUpVector();
		FVector targetDirection = finalDirections[i];

		if (targetDirection.IsNearlyZero()) continue;

		FVector SmoothedDir = FMath::VInterpNormalRotationTo(currentUp, targetDirection, DeltaTime, 5.0f);
		FQuat Delta = FQuat::FindBetween(currentUp, SmoothedDir);
		transforms[i].SetRotation(Delta * transforms[i].GetRotation());
		FVector newLoc = transforms[i].GetLocation() + (transforms[i].GetRotation().GetUpVector() * speed * DeltaTime);
		transforms[i].SetLocation(newLoc);
	}
	mesh->BatchUpdateInstancesTransforms(0, transforms, false, true);
}

FVector AInstanciateBoids::SeparateBoids(int index)
{
	FVector actualPosition = transforms[index].GetLocation();
	FVector separationForce = FVector::ZeroVector;
	for (int i = 0; i < numBoids; ++i)
	{
		if (i == index) continue;

		FVector diffBetweenAandB = actualPosition - transforms[i].GetLocation();
		float distSq = diffBetweenAandB.SizeSquared();
		if (distSq < maxDistance*maxDistance && distSq > 0.01f) 
		{
			float dist = FMath::Sqrt(distSq);
			float ratio = 1 - (dist/maxDistance);
			separationForce += (diffBetweenAandB / dist) * ratio;
		}
	}
	return separationForce;
}

void AInstanciateBoids::CalculateGlobalData()
{
	FVector alignSum = FVector::ZeroVector;
	FVector centerSum = FVector::ZeroVector;

	for (const FTransform& t : transforms)
	{
		alignSum += t.GetRotation().GetUpVector();
		centerSum += t.GetLocation();
	}

	alignment = alignSum.GetSafeNormal();
	if (numBoids > 0)
	{
		centroid = centerSum / numBoids;
	}
}

void AInstanciateBoids::SetBoidRotation(FTransform& transform, FVector direction) const
{
	FVector CurrentUp = transform.GetRotation().GetUpVector();
	if (CurrentUp.Equals(direction, 0.01f)) return;
	FQuat DeltaRotation = FQuat::FindBetween(CurrentUp, direction);
	transform.SetRotation(DeltaRotation * transform.GetRotation());
}

