// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Curves/CurveFloat.h"
#include "FireflyPetComponent.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFollowStartStopSignature, UFireflyPetComponent*, target);

UENUM(BlueprintType)
enum class EFollowType : uint8 {
	AtOwner,
	Leashing,
	FollowingTarget,
	WorldLocation
};



class UStickyTarget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UPSIDEDROWN_API UFireflyPetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFireflyPetComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
		FVector FDestinationPosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
		FVector FDestinationVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
		AActor* ADestinationActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
		USceneComponent* UDestinationComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
		float fLeashMaxRange = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
		float fLeashMinRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
		int iExtraGroundChecks = 5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
		EFollowType followType = EFollowType::AtOwner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
		bool bCloseToTarget = false;

	class UFlyingMovement* movementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Info")
		USceneComponent* idleTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool bDrawDebug = false;


	bool TargetReachedCheck();

	bool CheckForGroundCollision(FVector& desiredDirection);

	void UpdateDestination();

	virtual void StartMoving();

	void AssumeVelocity(FVector _velocity);
	const FVector GetNoraLocation() const;
	const FVector GetOwnLocation() const;

	bool RaycastForGround(FHitResult& result, FVector& scannedVector, const TArray<FVector>& normalizedVectors, FVector& _start, const float _raycastLength);

	FCollisionQueryParams mCollisionQueryParams = FCollisionQueryParams::DefaultQueryParam;


public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void SetFollowTarget(const FVector& destination);
	virtual void SetFollowTarget(AActor* targetActor);
	virtual void SetFollowTarget(USceneComponent* targetComponent);
	virtual void ReturnToOwner();

	UFUNCTION(BlueprintCallable, Category = "Event", meta = (DisplayName = "OnStickyTargetUpdate"))
		virtual void StickyTargetUpdate(UStickyTarget* const target);


	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "OnStartFollow"))
		FFollowStartStopSignature OnStartFollowDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "OnReachFollowTarget"))
		FFollowStartStopSignature OnReachFollowTargetDelegate;

	const EFollowType GetFollowType() const;
	AActor* GetFollowingObject() const;
};
