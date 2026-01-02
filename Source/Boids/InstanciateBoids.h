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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids Settings")
	float viewAngle = 180.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids Settings")
	int chunkSize = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids Settings")
	float alignWeight = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids Settings")
	float cohesionWeight = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids Settings")
	float separationWeight = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boids Settings")
	float avoidanceWeight = 2.0f;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	TArray<FVector> alignment;
	TArray<FVector> centroid;
	TArray<FVector> separation;
	TArray<FVector> alignementOfChunks;
	TArray<FVector> centroidOfChunks;
	TArray<FVector> finalDirections;
	TArray<TArray<int>> boidsInField;
	
	FVector SeparateBoids(int index);
	FVector ObjectAvoidance(int index);
	void FindBoidsInField();
	void CalculateGlobalData(int index);;

	void SetBoidRotation(FTransform& transform, FVector direction) const;
};
