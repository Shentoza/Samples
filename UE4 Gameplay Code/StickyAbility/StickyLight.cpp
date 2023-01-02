// Fill out your copyright notice in the Description page of Project Settings.

#include "StickyLight.h"
#include "Util/Util.h"
#include "DrawDebugHelpers.h"
#include "../FireflyPetComponent.h"
#include "StickyTarget.h"
#include "../TargetProvider.h"


// Sets default values for this component's properties
UStickyLight::UStickyLight()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	//ObjectQueryParams.AddObjectTypesToQuery();
	// ...
}


// Called when the game starts
void UStickyLight::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters spawnInfo;
	spawnInfo.Owner = GetOwner();
	spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	targetProvider = GetOwner()->FindComponentByClass<UTargetProvider>();
	if (targetProvider) {
		targetProvider->TargetChangedDelegate.AddDynamic(this, &UStickyLight::UpdateTarget);
		targetProvider->SetTargetingRange(fMaxRange);
	}

	for (int i = 0; i < iAmountOfFireflies; ++i) {
		const auto SpawnedFirefly = GetWorld()->SpawnActor<AActor>(objectsToSpawn[0], GetComponentLocation(), GetComponentRotation(), spawnInfo);
		const auto PetComponent = SpawnedFirefly->FindComponentByClass<UFireflyPetComponent>();
		if (PetComponent->IsValidLowLevelFast()) {
			OnStickyTargetChangeDelegate.AddDynamic(PetComponent, &UFireflyPetComponent::StickyTargetUpdate);
			fireflies.Add(PetComponent);
		}
	}
}



// Called every frame
void UStickyLight::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!targetProvider) {
		TArray<FHitResult> hits;
		const FVector start = GetCameraLocation();
		const FVector end = start + fMaxRange * GetAimDirection();
		GetWorld()->LineTraceMultiByChannel(hits, start, end, ECC_Camera, CollisionQueryParams);
		UStickyTarget* newTarget = nullptr;
		float ClosestDistance = TNumericLimits<float>::Max();
		for (const FHitResult& hit : hits) {
			if (auto targetComp = hit.GetActor()->FindComponentByClass<UStickyTarget>()) {
				if (bDrawDebug) { DrawDebugSphere(GetWorld(), hit.ImpactPoint, 100.0f, 10, FColor::Green); }

				if (auto dist = hit.Distance < ClosestDistance) {
					newTarget = targetComp;
					ClosestDistance = dist;
				}
			}
			else if (bDrawDebug) { DrawDebugSphere(GetWorld(), hit.ImpactPoint, 100.0f, 10, FColor::Green); }
		}
		UpdateTarget(newTarget->GetOwner());
	}
	CheckForMaxRange();


	if (OnAttachStayDelegate.IsBound()) {
		OnAttachStayDelegate.Broadcast(DeltaTime);
	}

	if (bPulling) {
		if (OnPullDelegate.IsBound()) {
			OnPullDelegate.Broadcast(DeltaTime);
		}
	}
	else if (bPushing) {
		if (OnPushDelegate.IsBound()) {
			OnPushDelegate.Broadcast(DeltaTime);
		}
	}
}

void UStickyLight::PushAction(const bool active) {
	bPushing = active;
	if (active) { bPulling = false; }
}

void UStickyLight::PullAction(const bool active) {
	bPulling = active;
	if (active) { bPushing = false; }
}

void UStickyLight::SendFirefly(UStickyTarget* target) {

	if (!target->IsValidLowLevelFast()) { target = currentTarget; }
	if (!target->IsValidLowLevelFast()) { return; }

	auto AObjectToAttach = target->GetOwner();
	if (AttachedObjects.Contains(AObjectToAttach)) { return; }

	for (auto firefly : fireflies)
	{
		if (firefly->GetFollowType() == EFollowType::AtOwner || firefly->GetFollowType() == EFollowType::Leashing) {
			firefly->SetFollowTarget(target);
			firefly->OnReachFollowTargetDelegate.AddDynamic(this, &UStickyLight::AttachFirefly);
			travelingFireflies.Add(firefly, AObjectToAttach);
			if (OnSendFireflyDelegate.IsBound()) {
				OnSendFireflyDelegate.Broadcast(AObjectToAttach->GetActorLocation());
			}
			break;
		}
	}
}

//Dinge für Targetcheck auskommentiert
void UStickyLight::RecallFirefly(UFireflyPetComponent* firefly) {

	AActor * AObjectToDetach = firefly->GetFollowingObject();
	if (!AttachedObjects.Contains(AObjectToDetach)) { return; }
	if (!AObjectToDetach->IsValidLowLevelFast()) { return; }

	firefly->ReturnToOwner();
	if (bHideOnAttach) {
		firefly->GetOwner()->SetActorHiddenInGame(false);
	}
	auto targets = GetTargetsInActor(AObjectToDetach);
	for (auto DetachingComponent : targets) {
		DetachingComponent->OnDetach();
	}
	AttachedObjects.RemoveSingleSwap(AObjectToDetach);

	if (OnRecallFireflyDelegate.IsBound()) {
		OnRecallFireflyDelegate.Broadcast(AObjectToDetach->GetActorLocation());
	}
}

//Dinge für Targetcheck auskommentiert
void UStickyLight::RecallAllFireflies()
{
	for (auto firefly : fireflies)
	{
		RecallFirefly(firefly);
	}
}

//Called when our sent out firefly arrives it's target
void UStickyLight::AttachFirefly(UFireflyPetComponent* const arrivedFirefly) {
	auto detachedObject = travelingFireflies.FindAndRemoveChecked(arrivedFirefly);

	if (AttachedObjects.AddUnique(detachedObject) != -1) {
		for (auto target : GetTargetsInActor(detachedObject)) {
			target->OnAttach(this);

		}
		arrivedFirefly->OnReachFollowTargetDelegate.RemoveDynamic(this, &UStickyLight::AttachFirefly);
		if (bHideOnAttach) {
			arrivedFirefly->GetOwner()->SetActorHiddenInGame(true);
		}
	}
}

const TArray<UStickyTarget*> UStickyLight::GetTargetsInActor(const AActor * aactor) const
{
	TArray<UStickyTarget*> result;
	for (auto i : aactor->GetComponentsByClass(targetClass)) {
		result.Add(Cast<UStickyTarget>(i));
	}
	return result;
}

const FVector UStickyLight::GetAimDirection() const {
	return Util::GetPlayerCameraManager(GetWorld())->GetActorForwardVector();
}

const FVector UStickyLight::GetCameraLocation() const {
	return Util::GetPlayerCameraManager(GetWorld())->GetCameraLocation();
}

const FVector UStickyLight::GetOwnerForward() const {
	return GetOwner()->GetActorForwardVector();
}

const FVector UStickyLight::GetOwnerLocation() const {
	return GetOwner()->GetActorLocation();
}

void UStickyLight::UpdateTarget(AActor* newTarget)
{
	UStickyTarget* targetComp = newTarget ? newTarget->FindComponentByClass<UStickyTarget>() : nullptr;
	if (targetComp != currentTarget) {
		if (OnStickyTargetChangeDelegate.IsBound()) {
			OnStickyTargetChangeDelegate.Broadcast(targetComp);
		}
		//Could be removed again
		if (currentTarget->IsValidLowLevel()) {
			currentTarget->OnTargeted(false);
		}
		currentTarget = targetComp;
		if (currentTarget->IsValidLowLevel()) {
			currentTarget->OnTargeted(true);
		}
	}
}

void UStickyLight::CheckForMaxRange()
{
	for (auto firefly : fireflies) {
		AActor * AObjectToDetach = firefly->GetFollowingObject();
		if (!AttachedObjects.Contains(AObjectToDetach)) { return; }
		if (!AObjectToDetach->IsValidLowLevelFast()) { return; }

		if (FVector::DistSquared(GetOwner()->GetActorLocation(), AObjectToDetach->GetActorLocation()) > (fMaxRange * fMaxRange)) {
			RecallFirefly(firefly);
		}
	}
}
