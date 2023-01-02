// Fill out your copyright notice in the Description page of Project Settings.

#include "TargetProvider.h"
#include "StickyAbility/StickyTarget.h"
#include "Util/Util.h"
#include "Util/DebugUtil.h"
#include "Util/MathUtil.h"
#include "UDPlayerController.h"
#include "ConvexHull2d.h"
#include "Util/Util.h"
#include "GameFramework/HUD.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UTargetProvider::UTargetProvider()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTargetProvider::BeginPlay()
{
	Super::BeginPlay();

	cameraManager = Util::GetPlayerCameraManager(GetWorld(), 0);
	playerController = Util::GetUDPlayerController(GetWorld());

}

// Called every frame
void UTargetProvider::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	UpdateScreenSize();
	TArray<FOverlapResult> OverResults;
	GetWorld()->OverlapMultiByChannel(OverResults, GetOwner()->GetActorLocation(), FQuat::Identity, ECC_GameTraceChannel3, FCollisionShape::MakeSphere(fMaxRange));

	float maxValue = 0.0f;
	AActor* bestActor = nullptr;
	//Parallel for?
	for (auto result : OverResults) {
		float value = EvaluateSingleTarget(result);
		if (value > maxValue) {
			maxValue = value;
			bestActor = result.GetActor();
		}
	}
	TargetFound(bestActor);
}



float UTargetProvider::EvaluateSingleTarget(FOverlapResult& _overlap) const
{
	const AActor* actor = _overlap.GetActor();
	if (!actor->IsValidLowLevelFast())
		return -1.0f;
	TArray<UActorComponent*> results = actor->GetComponentsByClass(TargetClass);
	//StickyTarget gefunden
	if (results.Num()) {
		FStickyTargetInfo targetInfo;
		targetInfo.targetActor = _overlap.GetActor();

		if (CheckVisibility(targetInfo)) {
			CreateScreenPolygon(targetInfo);
			if (!targetInfo.polygonVisible) {
				return 0.0f;
			}


			float DistanceToScreenMid = GetDistanceToMid(targetInfo);
			if (DistanceToScreenMid >= fMaxScreenDistance)
			{
				DistanceToScreenMid = 0.0f;
			}

			float ScreenArea = FMath::Clamp(GetScreenArea(targetInfo), 0.0f, fClampObjectSize);



			float targetPriority = FMath::Lerp(fScreenPosPriority*ScreenArea, 1.0f, 1.0f - DistanceToScreenMid);
			if (targetInfo.bCenteredInScreen) {
				targetPriority = 1.0f;
			}

			if (bDrawDebug) {
				DrawDebugString(GetWorld(), targetInfo.targetActor->GetActorLocation(), "ScreenArea :" + FString::SanitizeFloat(ScreenArea) + "\n DistanceToScreenMid: " + FString::SanitizeFloat(DistanceToScreenMid) + "\n Priority: "+FString::SanitizeFloat(targetPriority), NULL, FColor::Red, 0.01f);
			}
			return targetPriority;
		}
	}
	return -1.0f;
}

//Update Targets
void UTargetProvider::TargetFound(AActor* target)
{
	if (target != currentTarget) {
		currentTarget = target;
		if (TargetChangedDelegate.IsBound()) {
			TargetChangedDelegate.Broadcast(target);
		}
	}
}

bool UTargetProvider::CheckVisibility(FStickyTargetInfo& targetInfo) const
{
	//War gerade sichtbar, Occlusion culling nicht verlässlich
	if (!targetInfo.targetActor->WasRecentlyRendered())
		return false;

	FBoxSphereBounds actorBounds = targetInfo.targetActor->GetRootComponent()->Bounds;
	//Eckpunkte über alle -/+ Kombinationen von origin und half Extension vektor
	FHitResult hitresult;
	for (int i = -1; i < 2; i++) {
		for (int j = -1; j < 2; j++) {
			for (int k = -1; k < 2; k++) {
				FVector PositionChecked = actorBounds.Origin + (FVector((float)i, (float)j, (float)k) * actorBounds.BoxExtent);


				GetWorld()->LineTraceSingleByChannel(hitresult, GetCameraLocation(), PositionChecked, ECC_Visibility);
				if (hitresult.GetActor() == targetInfo.targetActor) {
					targetInfo.worldPointsVisible.Add(hitresult.ImpactPoint);
				}
			}
		}
	}

	return targetInfo.worldPointsVisible.Num();
}

void UTargetProvider::CreateScreenPolygon(FStickyTargetInfo& targetData) const
{
	FVector2D v2;

	for (auto v : targetData.worldPointsVisible) {
		playerController->ProjectWorldLocationToScreen(v, v2, true);
		//v2 *= FVector2D{ screenWidthInverse, screenHeightInverse};

		targetData.screenPointsVisible.Add(FVector{ v2.X, v2.Y, 0.0f });
		if (!(v2.X > screenWidth || v2.X < 0.0f || v2.Y > screenHeight || v2.Y < 0.0f)) {
			targetData.polygonVisible = true;
		}
	}
	if (!targetData.polygonVisible) {
		return;
	}

	TArray<int32> convexHullIndices;
	ConvexHull2D::ComputeConvexHull(targetData.screenPointsVisible, convexHullIndices);
	for (int32 i : convexHullIndices) {
		targetData.clippedScreenPolygon.Add(targetData.screenPointsVisible[i]);
	}

	TArray<FVector> result;
	MathUtil::ClipPolygonToScreenspace(targetData.clippedScreenPolygon, result);
	targetData.clippedScreenPolygon = result;

	convexHullIndices.Empty();
	ConvexHull2D::ComputeConvexHull(targetData.clippedScreenPolygon, convexHullIndices);
	result = targetData.clippedScreenPolygon;
	targetData.clippedScreenPolygon.Empty();
	for (int32 i : convexHullIndices) {
		targetData.clippedScreenPolygon.Add(result[i]);
	}

	if (bDrawDebug) {
		for (int i = 0; i < targetData.clippedScreenPolygon.Num(); ++i) {
			FVector dump;
			FVector current;
			FVector next;
			MathUtil::ScreenToWorld(GetWorld(), FVector2D{ targetData.clippedScreenPolygon[i].X, targetData.clippedScreenPolygon[i].Y }, current, dump, 0);
			MathUtil::ScreenToWorld(GetWorld(), FVector2D{ targetData.clippedScreenPolygon[(i + 1) % targetData.clippedScreenPolygon.Num()].X, targetData.clippedScreenPolygon[(i + 1) % targetData.clippedScreenPolygon.Num()].Y }, next, dump, 0);
			DrawDebugLine(GetWorld(), current, next, FColor::Red, false, -1.0f, 0, 0.1f);
		}
	}
}

//Berechnet die Fläche auf dem Bildschirm (Screenspace)
float UTargetProvider::GetScreenArea(FStickyTargetInfo& targetData) const
{
	float area = 0.0f;
	int indices = targetData.clippedScreenPolygon.Num();
	for (int i = 0; i < indices; ++i) {
		auto p1 = targetData.clippedScreenPolygon[i];
		auto p2 = targetData.clippedScreenPolygon[(i + 1) % indices];

		area += (p1.X * p2.Y) - (p1.Y * p2.X);
	}

	return FMath::Min(fabsf(area * 0.5f) / (screenWidth * screenHeight), 1.0f);
}

//Berechnet den Abstand zur Mitte des Bildschirms (Screenpos)
float UTargetProvider::GetDistanceToMid(FStickyTargetInfo& targetData) const
{
	//Wir benutzen immer nur  boxen!
	FBox box = targetData.targetActor->GetRootComponent()->Bounds.GetBox();
	targetData.bCenteredInScreen = false;
	FVector startToEnd = (GetCameraLocation() + fMaxRange * GetCameraForward()) - GetCameraLocation();

	if (FMath::LineBoxIntersection(box, GetCameraLocation(), GetCameraLocation() + fMaxRange * GetCameraForward(), startToEnd))
	{
		targetData.bCenteredInScreen = true;
		return 0.0f;
	}

	//Ansonsten Abstand zu nächster Kante
	FVector closestPoint;
	FVector secondClosestPoint;
	float minDistanceSquared = 2.0f;
	float secondMinDistanceSquared = 2.0f;


	float currentDistance = 0.0f;
	FVector screenMid{ 0.5f, 0.5f, 0.f };
	for (int i = 0; i < targetData.clippedScreenPolygon.Num(); ++i) {
		auto p = targetData.clippedScreenPolygon[i];
		p.X *= screenWidthInverse;
		p.Y *= screenHeightInverse;
		currentDistance = FVector::DistSquared(screenMid, p);

		//Könnte zweitbester Punkt sein
		if (currentDistance < secondMinDistanceSquared) {
			//Ist sogar bester Punkt
			if (currentDistance < minDistanceSquared) {
				secondClosestPoint = closestPoint;
				secondMinDistanceSquared = minDistanceSquared;

				closestPoint = p;
				minDistanceSquared = currentDistance;
			}
			else {
				secondClosestPoint = p;
				secondMinDistanceSquared = currentDistance;
			}
		}
	}
	FVector result = screenMid - FMath::ClosestPointOnSegment(screenMid, closestPoint, secondClosestPoint);
	return result.Size();
}

void UTargetProvider::UpdateScreenSize()
{
	int32 x, y;
	playerController->GetViewportSize(x, y);
	if (screenWidth != x || screenHeight != y) {
		screenWidth = x;
		screenWidthInverse = 1.0f / (float)x;

		screenHeight = y;
		screenHeightInverse = 1.0f / (float)y;
	}
}

//Checks all the bounding cornerpoints, if an object is visible, or maybe occluded

FVector UTargetProvider::GetCameraLocation() const
{
	if (cameraManager)
		return cameraManager->GetCameraLocation();
	return FVector::ZeroVector;
}

FVector UTargetProvider::GetCameraForward() const
{
	if (cameraManager->IsValidLowLevelFast())
		return cameraManager->GetActorForwardVector();
	return FVector::ZeroVector;
}

void UTargetProvider::SetTargetingRange(float fNewRange)
{
	fMaxRange = fNewRange;
}