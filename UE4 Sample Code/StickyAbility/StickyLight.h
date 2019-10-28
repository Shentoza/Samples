// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"
#include "Serialization/Serializable.h"
#include "Util/DebugUtil.h"
#include "StickyTarget.h"
#include "StickyLight.generated.h"


struct FTransform;
struct FActorSpawnParameters;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttachedFunctionSignature, float, DeltaTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStickyTargetChangeSignature, UStickyTarget*, target);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFireflyCommandSignature, FVector, targetLocation);

class UFireflyPetComponent;

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), HideCategories = ("Hidden"))
class UPSIDEDROWN_API UStickyLight : public USceneComponent, public ISerializable
{
	GENERATED_BODY()

	friend class ANoraPlayerCharacter;


public:	
	// Sets default values for this component's properties
	UStickyLight();

protected:

	UPROPERTY(SaveGame, VisibleAnywhere, BlueprintReadOnly, Category = "Ability Info")
	TArray<AActor*> AttachedObjects;

	TMap<UFireflyPetComponent*, AActor*> travelingFireflies;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Ability Info")
	UStickyTarget* currentTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Constraints")
	TArray<TSubclassOf<class AActor>> objectsToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Constraints")
		bool bHideOnAttach;


	TArray<UFireflyPetComponent*> fireflies;

	UFUNCTION(BlueprintCallable, Category = "StickyLight")
	virtual void SendFirefly(UStickyTarget* target);

	UFUNCTION(BlueprintCallable, Category = "StickyLight")
	virtual void RecallFirefly(UFireflyPetComponent* target);

	UFUNCTION(BlueprintCallable, Category = "StickyLight")
	virtual void RecallAllFireflies();

	UFUNCTION(BlueprintCallable, Category = "StickyLight")
		virtual void PushAction(const bool active) ;

	UFUNCTION(BlueprintCallable, Category = "StickyLight")
		virtual void PullAction(const bool active) ;

	void CheckForMaxRange();

	class UTargetProvider* targetProvider;


	// Called when the game starts
	virtual void BeginPlay() override;


	FCollisionObjectQueryParams ObjectQueryParams = FCollisionObjectQueryParams::DefaultObjectQueryParam;

	FCollisionQueryParams CollisionQueryParams = FCollisionQueryParams::DefaultQueryParam;

	UClass * targetClass = UStickyTarget::StaticClass();
	

public:	

	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "OnAttachedStay"))
	FAttachedFunctionSignature OnAttachStayDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "OnPush"))
	FAttachedFunctionSignature OnPushDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "OnPull"))
	FAttachedFunctionSignature OnPullDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "OnStickyTargetChange"))
	FStickyTargetChangeSignature OnStickyTargetChangeDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "OnSendFirefly"))
		FFireflyCommandSignature OnSendFireflyDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (DisplayName = "OnRecallFirefly"))
		FFireflyCommandSignature OnRecallFireflyDelegate;


	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "StickyLight")
	virtual void AttachFirefly(UFireflyPetComponent* const arrivedFirefly);

	/*
	Todo: Das als einzelne Actors verwalten (Array an Actors / unterschiedliche Fireflies) die man besitzt
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Constraints")
		int iAmountOfFireflies = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Constraints")
	float fMaxRange = 25000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Constraints")
	float collisionRadius = 500.f;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability Constraints")
	TEnumAsByte<ECollisionChannel> QueryChannels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = true;

	bool bPulling = false;
	bool bPushing = false;

	const TArray<UStickyTarget*> GetTargetsInActor(const AActor * aactor) const;

	const FVector GetAimDirection() const;

	const FVector GetCameraLocation() const;

	const FVector GetOwnerForward() const;

	const FVector GetOwnerLocation() const;

	UFUNCTION(BlueprintCallable)
	void UpdateTarget(AActor* newTarget);
};
