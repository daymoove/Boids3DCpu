#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InstanciateBoids.generated.h"

UCLASS()
class BOIDS_API AInstanciateBoids : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInstanciateBoids();

	//virtual void OnConstruction(const FTransform& Transform) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* mesh;

	UPROPERTY(VisibleAnywhere)
	TArray<FTransform> transforms;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids Settings")
	int numBoids = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids Settings")
	int speed = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids Settings")
	int maxDistance = 100;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FVector alignment;
	FVector centroid;
	TArray<FVector> finalDirections;
	void AlignBoids();
	FVector SeparateBoids(int index);
	void CalculateGlobalData();
	void CohesionBoids();

	void SetBoidRotation(FTransform& transform, FVector direction) const;
};
