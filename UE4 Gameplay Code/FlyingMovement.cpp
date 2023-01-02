// Fill out your copyright notice in the Description page of Project Settings.

#include "FlyingMovement.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Util/DebugUtil.h"
#include "GameFramework/Actor.h"


// Sets default values for this component's properties
UFlyingMovement::UFlyingMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

// Called when the game starts
void UFlyingMovement::BeginPlay()
{
	Super::BeginPlay();
	physicsComponent = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	if (!physicsComponent) {
		DebugUtil::DebugLog("UStaticMeshComponent required!");
	}
}


// Called every frame
void UFlyingMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ApplyRotation(DeltaTime);
	ApplyPosition(DeltaTime);

}

//Bewegen in Weltkoordinaten mit einer normalized auf 1 geclampten Richtung
void UFlyingMovement::Move(const FVector _inputDirection)
{
	FDesiredDirection = _inputDirection;
	if (_inputDirection.SizeSquared() >= 1.0f)
		FDesiredDirection.Normalize();
}

//Bewege dich in RIchtung, mit max Geschwindigkeit 0..1
void UFlyingMovement::Move(const FVector _inputDirection, const float _velocity) {
	if (_velocity < fMaxVelocity)
		FDesiredDirection = _inputDirection.GetClampedToMaxSize(_velocity);
	else
		Move(_inputDirection);
}

//Bewege dich in Richtung, nimm dabei folgende Geschwindigkeit an
void UFlyingMovement::Move(const FVector _inputDirection, const FVector _velocity)
{
	const float maxVelocity = _velocity.SizeSquared() / (fMaxVelocity*fMaxVelocity);
	Move(_inputDirection, maxVelocity);
}

void UFlyingMovement::ApplyPosition(const float DeltaTime)
{
	float desiredVelocity = fMaxVelocity * FDesiredDirection.Size();
	if (fVelocity < desiredVelocity) {
		fVelocity = FMath::Clamp(fVelocity + DeltaTime * fAcceleration,0.0f, desiredVelocity);
	}
	else {
		fVelocity = FMath::Clamp(fVelocity - DeltaTime * fDecceleration, desiredVelocity, fMaxVelocity);
	}
	physicsComponent->SetAllPhysicsLinearVelocity(GetVelocityVector());
}

void UFlyingMovement::ApplyRotation(const float DeltaTime)
{
	FVector normalizedDirection = FDesiredDirection.GetSafeNormal();

	fAngleDiff = FMath::RadiansToDegrees(acosf(FVector::DotProduct(normalizedDirection, GetOwnerForward())));
	bool RightSide = FVector::DotProduct(normalizedDirection, GetOwnerRight()) >= 0.0f;
	
	if (RightSide) {
		fAngleDiff *= -1.0f;
	}

	//Eventuell überflüssig nach grundsätzlichem Fixen der Drehung
	fAngleDiff = FMath::ClampAngle(fAngleDiff, -175.0f, 175.0f);
	//Wenn innerhalb der Toleranz für Winkel, wird die Richtung einfach hart gesetzt, und es wird keine Rotation mehr durchgeführt.
	if (fabs(fAngleDiff) < fRotationTolerance)
	{
		physicsComponent->SetWorldRotation(normalizedDirection.ToOrientationQuat());
		physicsComponent->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		fAngularVelocity = 0.0f;

	}
	//Wenn der Winkel jedoch zu groß ist, beginnen wir über die MeshComponent
	else {
		//Im parallelen Fall, ist das Kreuzprodukt nicht möglich
		if (FVector::Parallel(normalizedDirection, GetOwnerForward())) {
			Cross = GetOwnerUp();
		}
		else {
			Cross = FVector::CrossProduct(GetOwnerForward(),normalizedDirection);
			Cross.Normalize();
		}

		//DrawDebugLine(GetWorld(), GetOwnerLocation(), GetOwnerLocation() + Cross * 1000.0f, FColor::Green);
		fAngularVelocity = FMath::Clamp(fAngularVelocity + fAngularAcceleration * DeltaTime, 0.0f, fMaxAngularVelocity);
		physicsComponent->SetAllPhysicsAngularVelocityInDegrees(Cross * fAngularVelocity);
	}
}

const FVector UFlyingMovement::GetOwnerForward() const
{
	return GetOwner()->GetActorForwardVector();
}

const FVector UFlyingMovement::GetOwnerRight() const
{
	return GetOwner()->GetActorRightVector();
}

const FVector UFlyingMovement::GetOwnerUp() const
{
	return GetOwner()->GetActorUpVector();
}

const FVector UFlyingMovement::GetOwnerLocation() const
{
	return GetOwner()->GetActorLocation();
}

const float UFlyingMovement::GetBrakingDistance() const
{
	return (fVelocity * fVelocity) / (2.0f*fDecceleration);
}

const float UFlyingMovement::GetBrakingDistanceSquared() const
{
	const float distance = GetBrakingDistance();
	return distance*distance;
}

FVector UFlyingMovement::GetVelocityVector() const
{
	return fVelocity * GetOwnerForward();
}

const float& UFlyingMovement::GetVelocity() const
{
	return fVelocity;
}

