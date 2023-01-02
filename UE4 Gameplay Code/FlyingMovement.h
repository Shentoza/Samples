// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlyingMovement.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPSIDEDROWN_API UFlyingMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFlyingMovement();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	class UStaticMeshComponent* physicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
	FVector FDesiredDirection;
	//Velocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Properties")
		float fMaxVelocity = 800.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
	float fVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Properties")
	float fAcceleration = 650.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Properties")
	float fDecceleration = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Properties")
		float fMaxAngularVelocity = 400.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
		float fAngularVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Properties")
		float fAngularAcceleration = 360.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Properties")
	float fRotationTolerance = 15.0f;

	//Angles in Degree
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
	float fAngleDiff;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
		FVector Cross;

	virtual void ApplyRotation(const float DeltaTime);
	virtual void ApplyPosition(const float DeltaTime);

	const FVector GetOwnerForward() const;
	const FVector GetOwnerRight() const;
	const FVector GetOwnerLocation() const;
	const FVector GetOwnerUp() const;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Move(const FVector _inputDirection);
	virtual void Move(const FVector _inputDirection, const float velocity);
	virtual void Move(const FVector _inputDirection, const FVector _velocity);

	const float GetBrakingDistance() const;
	const float GetBrakingDistanceSquared() const;
	FVector GetVelocityVector() const;
	const float& GetVelocity() const;
};
