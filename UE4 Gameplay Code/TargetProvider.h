// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StickyAbility/StickyTarget.h"
#include "TargetProvider.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetChangedSignature, AActor*, target);

USTRUCT()
struct UPSIDEDROWN_API FStickyTargetInfo {
	GENERATED_BODY()

	UPROPERTY()
	bool bCenteredInScreen = false;

	UPROPERTY()
	bool polygonVisible = false;

	UPROPERTY()
	FVector closestPointToScreenCenter;

	UPROPERTY()
	TArray<FVector> worldPointsVisible;

	UPROPERTY()
	TArray<FVector> screenPointsVisible;

	UPROPERTY()
	TArray<FVector> clippedScreenPolygon;

	UPROPERTY()
	AActor* targetActor;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPSIDEDROWN_API UTargetProvider : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTargetProvider();



protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	UClass* TargetClass = UStickyTarget::StaticClass();

	class APlayerCameraManager* cameraManager;

	class AUDPlayerController* playerController;

	void TargetFound(AActor* target);

	AActor* currentTarget;

	float EvaluateSingleTarget(FOverlapResult& target) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float fMaxRange = 25000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float fScreenPosPriority = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float fClampObjectSize = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float fMaxScreenDistance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float fHitTolerance= 25.0f;

	bool CheckVisibility(FStickyTargetInfo& targetInfo) const;

	void CreateScreenPolygon(FStickyTargetInfo& targetData) const;
	float GetScreenArea(FStickyTargetInfo& targetData) const;
	float GetDistanceToMid(FStickyTargetInfo& targetData) const;

	int32 screenWidth;
	int32 screenHeight;
	float screenWidthInverse;
	float screenHeightInverse;

	void UpdateScreenSize();

	FVector GetCameraLocation() const;
	FVector GetCameraForward() const;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category ="Events", meta = (DisplayName = "OnTargetChange"))
	FTargetChangedSignature TargetChangedDelegate;

	void SetTargetingRange(float fNewRange);

};
