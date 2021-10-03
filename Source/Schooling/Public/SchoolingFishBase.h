// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SchoolingFishBase.generated.h"

UCLASS()
class SCHOOLING_API ASchoolingFishBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASchoolingFishBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FCollisionQueryParams NavigationCollisionQueryParams;

	UPROPERTY(Category = "Collision", EditAnywhere)
	float TurnFraction = 1.6180339f;

	UPROPERTY(Category = "Collision", EditAnywhere)
	float TraceLength = 500;

	UPROPERTY(Category = "Collision", EditAnywhere)
	float NumberOfTraces = 12;

	UPROPERTY(Category = "Schooling", EditAnywhere)
	FString GroupName = TEXT("Default");

	UPROPERTY(Category = "Movement", EditAnywhere)
	float MaxSpeed = 1000;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float MinSpeed = 30;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float DefaultSpeed = 400;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float Speed = 400;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float MaxAcceleration = 100;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float Acceleration = 0;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float Pitch = 0;
	
	UPROPERTY(Category = "Movement", EditAnywhere)
	float Yaw = 0;
	
	UPROPERTY(Category = "Movement", EditAnywhere)
	float Roll = 0;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float PitchStep = 30;
	
	UPROPERTY(Category = "Movement", EditAnywhere)
	float YawStep = 60;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float AccelerationStep = 20;

	TArray<AActor*> ActorsToIgnore;

	float LastNumberOfTraces = -1.0f;
	
	TArray<FVector> CollisionConeVectors;

	FTimerHandle TriggerUpdateAccelerationTimerHandle;

	FTimerHandle TriggerUpdateDirectionTimerHandle;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Category = "Mesh", VisibleDefaultsOnly)
	class USceneComponent* RootScene;

	UPROPERTY(Category = "Mesh", VisibleDefaultsOnly)
	class UStaticMeshComponent* Mesh;

	virtual void SetAccelerationTimer();

	virtual void SetDirectionalTimer();

	virtual void SetRandomDirection();

	virtual void SetRandomAcceleration();
	
	virtual void UpdateSpeed();

	virtual bool OffsetForCollisions(float DeltaTime);

	virtual bool IsObstacle(FHitResult hitResult);

	virtual void ExecuteMovement(float DeltaTime);

	virtual void UpdateLocation(float DeltaTime);
	
	virtual void UpdateDirection(float DeltaTime);

	virtual void ApplyDirectionalOffset(float DeltaTime);

	virtual float GetRandomFromRange(float min, float max);

	virtual int GetRandomFromRange(int min, int max);

	virtual void CheckCollisions(int numberOfTraces, FVector startLocation, FVector actorForwardVector,  TArray<FVector>& missedVectors, TArray<FVector>& hitVectors);

	virtual bool CheckCollision(FVector startLocation, FVector endLocation, FVector& hitStart);

	virtual void BuildEndLocationList(int numberOfTraces);

	virtual void UpdateEndLocationList(int numberOfTraces);

	virtual FVector SelectRandomItemFromArray(TArray<FVector> vectors);

	virtual FVector SelectFarthestVector(FVector location, TArray<FVector> vectors);

	virtual void ChangeDirectionTowardVector(FVector vector);
};