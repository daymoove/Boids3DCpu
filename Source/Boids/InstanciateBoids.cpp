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
	alignment.SetNumZeroed(numBoids);
	centroid.SetNumZeroed(numBoids);
	if (mesh->GetInstanceCount() == 0)
	{
		transforms.Empty(numBoids);
		for (int i = 0; i < numBoids; ++i)
		{
			transforms.Add(FTransform(FVector(FMath::RandRange(-2000,2000), FMath::RandRange(-2000,2000), FMath::RandRange(0,2000))));
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

	FindBoidsInField();
	
	ParallelFor(numBoids, [&](int i)
	{
		CalculateGlobalData(i);
		FVector separationForce = SeparateBoids(i);
		FVector cohesionForce = (centroid[i] - transforms[i].GetLocation()).GetSafeNormal();
		FVector objectAvoidance = ObjectAvoidance(i);
		FVector desiredDir = (alignment[i] * alignWeight) + (separationForce * separationWeight) + (cohesionForce * cohesionWeight) + (objectAvoidance * avoidanceWeight);
		finalDirections[i] = desiredDir.GetSafeNormal();
	});
	for (int i = 0; i < numBoids; ++i)
	{
		FVector currentUp = transforms[i].GetRotation().GetUpVector();
		FVector targetDirection = finalDirections[i];

		if (targetDirection.IsNearlyZero()) continue;

		//FVector SmoothedDir = FMath::VInterpNormalRotationTo(currentUp, targetDirection, DeltaTime, 60.0f);
		FQuat Delta = FQuat::FindBetween(currentUp, targetDirection);
		transforms[i].SetRotation(Delta * transforms[i].GetRotation());
		FVector newLoc = transforms[i].GetLocation() + (transforms[i].GetRotation().GetUpVector() * speed * DeltaTime);
		newLoc.Normalize();
		transforms[i].SetLocation(FMath::VInterpNormalRotationTo(transforms[i].GetLocation(), newLoc, DeltaTime, 90.0f));
	}
	mesh->BatchUpdateInstancesTransforms(0, transforms, false, true);
}

FVector AInstanciateBoids::SeparateBoids(int index)
{
	FVector actualPosition = transforms[index].GetLocation();
	FVector separationForce = FVector::ZeroVector;
	int count = 0;
	for (const int boidIndex : boidsInField[index])
	{
		if (boidIndex == index) continue;
		
		const float distBetweenAandB =  FVector::Dist(actualPosition, transforms[boidIndex].GetLocation());
		if (distBetweenAandB < chunkSize  && distBetweenAandB > 0)
		{
			FVector diffBetweenAandB = actualPosition - transforms[boidIndex].GetLocation();
			diffBetweenAandB.Normalize();
			float ratio = chunkSize / FMath::Max(distBetweenAandB, 1.0f);
			separationForce += diffBetweenAandB * ratio;
			count++;
		}
	}
	if (count > 0)
	{
		separationForce /= static_cast<float>(count);
		if (!separationForce.IsNearlyZero())
		{
			separationForce.Normalize();
		}
	}
	return separationForce;
}

FVector AInstanciateBoids::ObjectAvoidance(int index)
{
	int numPoints = 100;
	float goldenRatio = (1 + FMath::Sqrt(5.0f)) / 2.0f;
	for (int i = 0; i < numBoids; ++i)
	{
		float Theta = acos(1 - 2*(i / numPoints));
		float Phi = 360 * goldenRatio * i;
		FVector coords  = FVector::ZeroVector;
		coords.X = FMath::Cos(Phi) * FMath::Sin(Theta);
		coords.Y = FMath::Sin(Phi) * FMath::Sin(Theta);
		coords.Z = FMath::Cos(Theta);
		FVector rayStart = transforms[index].GetLocation();
		FVector rayEnd = rayStart + coords * maxDistance;
		FHitResult hitResult;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		bool isHit = GetWorld()->LineTraceSingleByChannel(hitResult, rayStart, rayEnd, ECC_Visibility, params);
		if (!isHit)
		{
			return coords.GetSafeNormal();
		}
	}
	return FVector::ZeroVector;
}

void AInstanciateBoids::CalculateGlobalData(int index)
{
	FVector alignSum = FVector::ZeroVector;
	FVector centerSum = FVector::ZeroVector;
	int numBoidsInField = boidsInField[index].Num();

	for (const int boidIndex : boidsInField[index])
	{
		const FTransform& t = transforms[boidIndex];
		alignSum += t.GetRotation().GetUpVector();
		centerSum += t.GetLocation();
	}

	alignment[index] = alignSum.GetSafeNormal();
	if (numBoidsInField > 0)
	{
		centroid[index] = centerSum / numBoidsInField;
	}
}

void AInstanciateBoids::SetBoidRotation(FTransform& transform, FVector direction) const
{
	FVector CurrentUp = transform.GetRotation().GetUpVector();
	if (CurrentUp.Equals(direction, 0.01f)) return;
	FQuat DeltaRotation = FQuat::FindBetween(CurrentUp, direction);
	transform.SetRotation(DeltaRotation * transform.GetRotation());
}
void AInstanciateBoids::FindBoidsInField()
{
	boidsInField.Empty();
	boidsInField.SetNum(numBoids);
	
	const float chunkRadius = chunkSize*chunkSize;
	const float dotPrductFOV = FMath::Cos(FMath::DegreesToRadians(viewAngle * 0.5f));
	TArray<TArray<int>> TempBoidsList;
	TempBoidsList.SetNum(numBoids);
	
	ParallelFor(numBoids, [&](const int32 i)
	{
		const FVector& actualPosition = transforms[i].GetLocation();
		const FVector& actualDirection = transforms[i].GetRotation().GetUpVector();
		
		for (int j = 0; j < numBoids; j++)
		{
			if (i==j)continue;
			if (chunkRadius > FVector::DistSquared(actualPosition, transforms[j].GetLocation()))
			{
				FVector directionToOther = (transforms[j].GetLocation() - actualPosition).GetSafeNormal();
				if (FVector::DotProduct(actualDirection, directionToOther) > dotPrductFOV)
				{
					TempBoidsList[i].Add(j);
				}
			}
		}
	});
	for (int i = 0; i < numBoids; i++)
	{
		boidsInField[i] = TempBoidsList[i];
	}
}

