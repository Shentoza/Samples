// Fill out your copyright notice in the Description page of Project Settings.

#include "FireflyPetComponent.h"
#include "Util/MathUtil.h"
#include "StickyAbility/StickyTarget.h"
#include "../Misc/Movement/FlyingMovement.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Util/DebugUtil.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Util/MathUtil.h"


// Sets default values for this component's properties
UFireflyPetComponent::UFireflyPetComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UFireflyPetComponent::BeginPlay()
{
	Super::BeginPlay();
	//TODO, component vermutlich an Nora hängen, kreiselbewegung fertig machen
	movementComponent = GetOwner()->FindComponentByClass<UFlyingMovement>();
	UStaticMeshComponent* mesh = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	mCollisionQueryParams.AddIgnoredActor(GetOwner()->GetOwner());
	mCollisionQueryParams.AddIgnoredActor(GetOwner());
	if (mesh)
		for (auto comp : mesh->GetAttachChildren()) {
			if (comp->GetName().Equals("CirclingComponent")) {
				idleTransform = comp;
			}
		}
}

// Called every frame
void UFireflyPetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateDestination();
	TargetReachedCheck();
	FVector desiredDirection = (FDestinationPosition - GetOwnLocation()).GetUnsafeNormal();

	//TODO: Check ersetzbar durch: FDestinationVelocity gesetzt?
	if (bCloseToTarget) {
		movementComponent->Move(desiredDirection, FDestinationVelocity);
	}
	else {
		CheckForGroundCollision(desiredDirection);
		movementComponent->Move(desiredDirection);
	}

	//Draw desired Direction
	if (bDrawDebug) {
		DrawDebugLine(GetWorld(), GetOwnLocation(), GetOwnLocation() + desiredDirection * 100.0f, FColor::Red, true, 0.01f);
	}

}

void UFireflyPetComponent::ReturnToOwner()
{
	UDestinationComponent = nullptr;
	ADestinationActor = nullptr;
	followType = EFollowType::AtOwner;
	StartMoving();
}

void UFireflyPetComponent::StickyTargetUpdate(UStickyTarget* const target)
{
	if (followType == EFollowType::FollowingTarget || followType == EFollowType::WorldLocation)
		return;

	UDestinationComponent = target;
	ADestinationActor = nullptr;
	if (target->IsValidLowLevelFast())
		followType = EFollowType::Leashing;
	else
		followType = EFollowType::AtOwner;

	StartMoving();
}

//Todo Refactor Targeting Methods
void UFireflyPetComponent::UpdateDestination()
{
	FVector FCurrentPosition = GetOwnLocation();
	FVector FNoraPosition = GetNoraLocation();
	FVector DiffVector;
	bCloseToTarget = false;
	switch (followType) {

		//Irrlicht soll versuchen richtung ziel zu fliegen, aber trotzdem in der Nähe des Ziels zu bleiben

		case EFollowType::Leashing:
		{
			if (UDestinationComponent->IsValidLowLevelFast()) {
				FDestinationPosition = UDestinationComponent->GetComponentLocation();
			}
			else if (ADestinationActor->IsValidLowLevelFast()) {
				FDestinationPosition = ADestinationActor->GetActorLocation();
			}
			DiffVector = (FDestinationPosition - FNoraPosition).GetClampedToMaxSize(fLeashMaxRange);
			FDestinationPosition = (FNoraPosition + DiffVector);
			if ((FDestinationPosition - FCurrentPosition).SizeSquared() <= movementComponent->GetBrakingDistanceSquared())
			{
				AssumeVelocity(GetOwner()->GetOwner()->GetVelocity());
			}
			break;
		}


		//Irrlicht folgt einem anderen Ziel
		case EFollowType::FollowingTarget: {
			bool targetIsActor = false;
			if (UDestinationComponent->IsValidLowLevelFast()) {
				FDestinationPosition = UDestinationComponent->GetComponentLocation();
			}
			else if (ADestinationActor->IsValidLowLevelFast()) {
				FDestinationPosition = ADestinationActor->GetActorLocation();
				targetIsActor = true;
			}
			DiffVector = (FDestinationPosition - FCurrentPosition);
			if (DiffVector.SizeSquared() - (fLeashMinRange * fLeashMinRange)  <= movementComponent->GetBrakingDistanceSquared()) {
				AssumeVelocity(targetIsActor ? ADestinationActor->GetVelocity() : UDestinationComponent->GetOwner()->GetVelocity());
			}
			break;
		}

										   //Fly to a fixed point in the world
		case EFollowType::WorldLocation: {
			DiffVector = FDestinationPosition - FCurrentPosition;
			if (movementComponent->GetBrakingDistanceSquared() <= DiffVector.SizeSquared()) {
				AssumeVelocity(FVector::ZeroVector);
			}
			else {
				bCloseToTarget = false;
			}
			break;
		}


		case EFollowType::AtOwner:
		default: {

			//Return
			FDestinationPosition = FNoraPosition;
			DiffVector = FDestinationPosition - FCurrentPosition;
			float distance = DiffVector.SizeSquared();

			if (distance - movementComponent->GetBrakingDistanceSquared() > fLeashMinRange * fLeashMinRange) {
				bCloseToTarget = false;
				FDestinationPosition = FNoraPosition;
			}
			//Ist beim Owner
			else {
				//TODO hier weiter machen!
				if (idleTransform) {
					const FVector idlePos = GetNoraLocation() + idleTransform->GetRelativeTransform().GetLocation();
					float dp = FVector::Dist(idlePos, FCurrentPosition);
					DebugUtil::DebugLog("##############");
					DebugUtil::DebugLog(idleTransform->GetComponentVelocity().ToCompactString());
					DebugUtil::DebugLog(FString::SanitizeFloat(dp));
					if (dp <= 50.0f) {
						GetOwner()->SetActorLocation(idlePos);
						AssumeVelocity(GetOwner()->GetOwner()->GetVelocity());
						DebugUtil::DebugLog("Points near!");
					}
					else {
						FDestinationPosition = idlePos;
						DiffVector = FDestinationPosition - FCurrentPosition;
						distance = DiffVector.SizeSquared();
						if (distance - movementComponent->GetBrakingDistanceSquared() <= 0.0f) {
							AssumeVelocity(FVector::ZeroVector);
						}
					}
				}
				else {
					AssumeVelocity(GetOwner()->GetOwner()->GetVelocity());
				}
			}
			break;
		}
	}
}

void UFireflyPetComponent::AssumeVelocity(FVector _velocity)
{
	bCloseToTarget = true;
	FDestinationVelocity = _velocity;
	FDestinationPosition = GetOwnLocation() + FDestinationVelocity;
}

void UFireflyPetComponent::StartMoving()
{
	if (OnStartFollowDelegate.IsBound())
		OnStartFollowDelegate.Broadcast(this);
}

//Todo targetreached check hier einbauen, raus aus update Destination
bool UFireflyPetComponent::TargetReachedCheck()
{
	bool finishedMoving = false;
	FVector pos = ADestinationActor ? ADestinationActor->GetActorLocation() : UDestinationComponent ? UDestinationComponent->GetComponentLocation() : FVector::ZeroVector;
	//todo: parametrisieren
	finishedMoving = FVector::PointsAreNear(pos, GetOwnLocation(), 150.0f);

	if (finishedMoving) {
		if (OnReachFollowTargetDelegate.IsBound()) {
			OnReachFollowTargetDelegate.Broadcast(this);
		}
	}
	return finishedMoving;
}

//Checkt ob unter (0,0,-1) oder vor(forwardVector) eine Bodenkollision stattfindet.
//Wenn gewünscht können noch mehrere extra Interpolationen durchgeführt werden.
bool UFireflyPetComponent::CheckForGroundCollision(FVector& desiredDirection)
{
	FVector Start = GetOwnLocation();

	//Eventuell zu frühes abbremsen?
	const float brakingDistance = movementComponent->GetBrakingDistance();
	FVector startingCheck = GetOwner()->GetActorForwardVector();
	FVector endCheck = -GetOwner()->GetActorUpVector();


	TArray<FVector> scanningVectors;
	scanningVectors.Add(startingCheck);
	//Fügt NLERPte Vektoren zwischen Down und Forward vektor ein
	for (int i = 0; i < iExtraGroundChecks; ++i) {
		float alpha = (float)(i + 1) / (float)(iExtraGroundChecks + 1);
		FVector interpVector = FMath::Lerp(startingCheck, endCheck, alpha).GetSafeNormal();
		scanningVectors.Add(interpVector);
	}

	FHitResult resultingHit;
	FVector resultingScan;
	//Berechne die Abänderung des Kurses
	if (RaycastForGround(resultingHit, resultingScan, scanningVectors, Start, brakingDistance)) {

		float alpha = 1.0f - (resultingHit.Distance / brakingDistance);
		float alpha2 = FVector::DotProduct(-resultingHit.Normal, startingCheck);
		FVector safePos = (Start + startingCheck + resultingHit.Normal);
		desiredDirection = FMath::Lerp(desiredDirection, (safePos - Start).GetUnsafeNormal(), FMath::Max(alpha2,alpha)).GetUnsafeNormal();

		return true;
	}
	return false;
}

bool UFireflyPetComponent::RaycastForGround(FHitResult& result, FVector& scannedVector, const TArray<FVector>& normalizedVectors, FVector& _start, const float _raycastLength) {
	TArray<FHitResult> hitResults;
	int i = 0;
	for (auto currentVector : normalizedVectors) {
		FVector currentEndpoint = _start + currentVector * _raycastLength;

		if (bDrawDebug)
			DrawDebugLine(GetWorld(), _start, currentEndpoint, FColor::Cyan, true, 0.05f);

		//Gather all hits that our checking rays get
		if (GetWorld()->LineTraceMultiByChannel(hitResults, _start, currentEndpoint, ECC_GameTraceChannel3, mCollisionQueryParams)) {
			for (FHitResult hit : hitResults) {
				if (hit.Actor->Tags.Contains("Ground")) {
					if (i == 0) {
					result = hit;
					scannedVector = currentVector;
					}
					else {
						scannedVector += currentVector;
						result.Distance += hit.Distance;
						result.Normal += hit.Normal;
					}
					++i;
				}
			}
		}
	}
	if (i > 1) {
		scannedVector.GetUnsafeNormal();
		result.Distance /= (float)i;
		result.Normal.GetUnsafeNormal();
	}
	return (i > 0);
}

void UFireflyPetComponent::SetFollowTarget(FVector const & destination)
{
	ADestinationActor = nullptr;
	UDestinationComponent = nullptr;
	FDestinationPosition = destination;
	followType = EFollowType::WorldLocation;
	StartMoving();
}

void UFireflyPetComponent::SetFollowTarget(AActor * targetActor)
{
	ADestinationActor = targetActor;
	UDestinationComponent = nullptr;
	followType = EFollowType::FollowingTarget;
	StartMoving();
}

void UFireflyPetComponent::SetFollowTarget(USceneComponent* targetComponent)
{

	ADestinationActor = nullptr;
	UDestinationComponent = targetComponent;
	followType = EFollowType::FollowingTarget;
	StartMoving();
}

const EFollowType UFireflyPetComponent::GetFollowType() const
{
	return followType;
}

AActor* UFireflyPetComponent::GetFollowingObject() const
{
	if (ADestinationActor->IsValidLowLevelFast()) {
		return ADestinationActor;
	}
	else if (UDestinationComponent->IsValidLowLevelFast()) {
		return UDestinationComponent->GetOwner();
	}
	return nullptr;
}

const FVector UFireflyPetComponent::GetNoraLocation() const
{
	return GetOwner()->GetOwner()->GetActorLocation();
}

const FVector UFireflyPetComponent::GetOwnLocation() const
{
	return GetOwner()->GetActorLocation();
}