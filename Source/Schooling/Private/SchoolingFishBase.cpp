// Fill out your copyright notice in the Description page of Project Settings.

#include "SchoolingFishBase.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ASchoolingFishBase::ASchoolingFishBase()
{
	//Create Root
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootScene;

	//Create static mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(Cast<AActor>(this->Mesh));
	ActorsToIgnore.Add(Cast<AActor>(this->RootScene));

	NavigationCollisionQueryParams = FCollisionQueryParams();
	NavigationCollisionQueryParams.AddIgnoredActors(this->ActorsToIgnore);

	this->BuildEndLocationList(this->NumberOfTraces);

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASchoolingFishBase::BeginPlay()
{
	Super::BeginPlay();

	// this->SetAccelerationTimer();

	// this->SetDirectionalTimer();
}

// Called every frame
void ASchoolingFishBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ExecuteMovement(DeltaTime);
}

void ASchoolingFishBase::ExecuteMovement(float DeltaTime)
{
	UpdateDirection(DeltaTime);

	UpdateSpeed();

	UpdateLocation(DeltaTime);
}

void ASchoolingFishBase::UpdateSpeed()
{
	this->Speed = FMath::Max(this->MinSpeed, FMath::Min(this->Speed + this->Acceleration, this->MaxSpeed));
}

void ASchoolingFishBase::UpdateLocation(float DeltaTime)
{
	FVector location = this->GetActorLocation();

	FVector direction = this->GetActorForwardVector();

	location += direction * Speed * DeltaTime;

	SetActorLocation(location);
}

void ASchoolingFishBase::UpdateDirection(float DeltaTime)
{
	this->OffsetForCollisions(DeltaTime);

	this->ApplyDirectionalOffset(DeltaTime);
}

bool ASchoolingFishBase::OffsetForCollisions(float DeltaTime)
{
	FVector actorForwardVector = GetActorForwardVector();
	FVector start = this->GetActorLocation();

	TArray<FVector> missedVectors;
	TArray<FVector> hitVectors;

	FVector hitStart;
	if (this->CheckCollision(start, start + actorForwardVector * this->TraceLength, hitStart))
	{
		FVector selectedVector = FVector::ZeroVector;

		if (!LastCollisionAvoidanceVector.IsZero())
		{
			selectedVector = this->LastCollisionAvoidanceVector;
		}

		this->CheckCollisions(this->NumberOfTraces, start, actorForwardVector, missedVectors, hitVectors);

		//If we're in route of collision with any of the traces, select one of the traces that didn't hit
		// If all the traces hit, select the trace that hit the farthest
		if (hitVectors.Num() > 0)
		{
			if (missedVectors.Num() > 0)
			{
				selectedVector = this->SelectRandomItemFromArray(missedVectors);
			}
			else
			{
				selectedVector = this->SelectFarthestVector(start, hitVectors);
			}

			DrawDebugLine(GetWorld(), start, selectedVector, FColor::Yellow);
		}

		if (!selectedVector.IsZero())
		{
			this->ChangeDirectionTowardVector(start, selectedVector);
			this->LastCollisionAvoidanceVector = selectedVector;

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		this->Pitch = 0;
		this->Yaw = 0;
		LastCollisionAvoidanceVector = FVector::ZeroVector;
	}

	return false;
}

void ASchoolingFishBase::ChangeDirectionTowardVector(FVector start, FVector target)
{
	FVector forward = (target - start);

	UKismetMathLibrary::GetYawPitchFromVector(forward, this->Yaw, this->Pitch);
}

FVector ASchoolingFishBase::SelectFarthestVector(FVector location, TArray<FVector> vectors)
{
	FVector selectedVector = location;
	float selectedDistance = 0.0f;

	for (int index = 0; index < vectors.Num(); index++)
	{
		FVector currentVector = vectors[index];

		float distance = FVector::Dist(location, currentVector);

		if (distance > selectedDistance)
		{
			selectedDistance = distance;
			selectedVector = currentVector;
		}
	}

	return selectedVector;
}

FVector ASchoolingFishBase::SelectRandomItemFromArray(TArray<FVector> vectors)
{
	int selectedIndex = this->GetRandomFromRange(0, vectors.Num() - 1);

	return vectors[selectedIndex];
}

void ASchoolingFishBase::CheckCollisions(int numberOfTraces, FVector startLocation, FVector actorForwardVector, TArray<FVector> &missedVectors, TArray<FVector> &hitVectors)
{
	this->UpdateEndLocationList(numberOfTraces);

	for (int index = 0; index < this->CollisionConeVectors.Num(); index++)
	{
		FVector vector = this->CollisionConeVectors[index];

		FVector endLocation = startLocation + (actorForwardVector + vector) * this->TraceLength;

		FVector hitStart;
		if (this->CheckCollision(startLocation, endLocation, hitStart))
		{
			hitVectors.Add(endLocation);
		}
		else
		{
			missedVectors.Add(endLocation);
		}
	}
}

void ASchoolingFishBase::UpdateEndLocationList(int numberOfTraces)
{
	if (numberOfTraces != this->LastNumberOfTraces)
	{
		this->BuildEndLocationList(this->NumberOfTraces);
	}
}

void ASchoolingFishBase::BuildEndLocationList(int numberOfTraces)
{
	this->CollisionConeVectors.Empty();
	for (int i = 0; i < numberOfTraces; i++)
	{
		float t = i / (numberOfTraces - 1.0f);
		float inclination = FMath::Acos(1 - 2 * t);
		float azimuth = 2 * PI * this->TurnFraction * i;

		float x = FMath::Sin(inclination) * FMath::Cos(azimuth);
		float y = FMath::Sin(inclination) * FMath::Sin(azimuth);
		float z = FMath::Cos(inclination);

		this->CollisionConeVectors.Add(FVector(x, y, z));
	}

	this->LastNumberOfTraces = numberOfTraces;
}

bool ASchoolingFishBase::CheckCollision(FVector startLocation, FVector endLocation, FVector &hitStart)
{
	FHitResult hitResult;

	bool isObstacle = false;

	if (GetWorld()->LineTraceSingleByChannel(hitResult, startLocation, endLocation, ECC_Camera, NavigationCollisionQueryParams))
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0, FColor::Orange, FString::Printf(TEXT("%s"), *hitResult.GetActor()->GetName()));

		isObstacle = this->IsObstacle(hitResult);

		if (isObstacle)
		{
			isObstacle = true;
		}
	}

	DrawDebugLine(GetWorld(), startLocation, endLocation, isObstacle ? FColor::Red : FColor::Green);

	return isObstacle;
}

bool ASchoolingFishBase::IsObstacle(FHitResult hitResult)
{
	return hitResult.bBlockingHit;
}

void ASchoolingFishBase::ApplyDirectionalOffset(float DeltaTime)
{
	FRotator NewRotation = FRotator(this->Pitch * DeltaTime, this->Yaw * DeltaTime, this->Roll * DeltaTime);

	FQuat QuatRotation = FQuat(NewRotation);

	AddActorLocalRotation(QuatRotation, false, 0, ETeleportType::None);
}

void ASchoolingFishBase::SetAccelerationTimer()
{
	float countdownStart = GetRandomFromRange(0.3f, 1.0f);

	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange, FString::Printf(TEXT("Countdown to acceleration randomization in %g seconds"), countdownStart));

	GetWorld()->GetTimerManager().SetTimer(TriggerUpdateAccelerationTimerHandle, this, &ASchoolingFishBase::SetRandomAcceleration, countdownStart, false);
}

void ASchoolingFishBase::SetDirectionalTimer()
{
	float countdownStart = GetRandomFromRange(0.2f, 0.6f);

	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, FString::Printf(TEXT("Countdown to acceleration randomization in %g seconds"), countdownStart));

	GetWorld()->GetTimerManager().SetTimer(TriggerUpdateDirectionTimerHandle, this, &ASchoolingFishBase::SetRandomDirection, countdownStart, false);
}

void ASchoolingFishBase::ResetDirectionalTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(TriggerUpdateDirectionTimerHandle);

	this->SetDirectionalTimer();
}

void ASchoolingFishBase::SetRandomAcceleration()
{
	// if (this->Acceleration != 0 && this->GetRandomFromRange(0, 2) >= 1)
	// {
	// 	this->Acceleration = 0 - this->Acceleration;
	// }
	// else
	// {
	// 	float randomAccelerationModifier = this->GetRandomFromRange(0 - this->AccelerationStep, this->AccelerationStep);
	// 	this->Acceleration = FMath::Max(0.0f, FMath::Min(this->Acceleration + randomAccelerationModifier, this->MaxAcceleration));
	// }

	// GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange, FString::Printf(TEXT("Set acceleration to %g!"), this->Acceleration));

	// this->SetAccelerationTimer();
}

void ASchoolingFishBase::SetRandomDirection()
{
	if (this->Chance(3))
	{
		this->Pitch = 0;
	}
	else
	{
		float randomPitchChange = this->GetRandomFromRange(0 - this->PitchStep, this->PitchStep);
		this->Pitch = randomPitchChange;
	}

	if (this->Chance(3))
	{
		this->Yaw = 0;
	}
	else
	{
		float randomYawChange = this->GetRandomFromRange(0 - this->YawStep, this->YawStep);
		this->Yaw = randomYawChange;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange, FString::Printf(TEXT("Set pitch to %g!"), this->Pitch));
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Orange, FString::Printf(TEXT("Set yaw to %g!"), this->Yaw));

	this->SetDirectionalTimer();
}

float ASchoolingFishBase::GetRandomFromRange(float min, float max)
{
	return FMath::RandRange(min, max);
}

int ASchoolingFishBase::GetRandomFromRange(int min, int max)
{
	return FMath::RandRange(min, max);
}

bool ASchoolingFishBase::Chance(int outOf)
{
	return FMath::RandRange(1, outOf) == outOf;
}