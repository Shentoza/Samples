// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "StickyTarget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPushPullSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttachDelegateSignature, bool, value);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UPSIDEDROWN_API UStickyTarget : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStickyTarget();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnAttach(class UStickyLight* attachingComponent);

	virtual void OnDetach();

	UFUNCTION(BlueprintCallable, Category = "Event", meta = (DisplayName = "OnAttachedStay"))
		virtual void OnAttachStay(const float DeltaTime);


	UFUNCTION(BlueprintCallable, Category = "Event", meta = (DisplayName = "OnPush"))
		virtual void OnPush(const float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Event", meta = (DisplayName = "OnPull"))
		virtual void OnPull(const float DeltaTime);

	virtual void OnTargeted(const bool targetActive);


	UFUNCTION(BlueprintCallable)
		void SetTargetActive(bool active) { bTargetActive = active; };

	UFUNCTION(BlueprintCallable)
		bool IsTargetActive() const { return bTargetActive; }

	UFUNCTION(BlueprintCallable)
		bool IsAttached() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Upside Drown", meta = (DisplayName = "OnPushEvent"))
		void ReceiveOnPush(const float DeltaTime);
	UFUNCTION(BlueprintImplementableEvent, Category = "Upside Drown", meta = (DisplayName = "OnPullEvent"))
		void ReceiveOnPull(const float DeltaTime);
	UFUNCTION(BlueprintImplementableEvent, Category = "Upside Drown", meta = (DisplayName = "OnAttachDetachFireflyEvent"))
		void ReceiveOnAttachedStay(const float DeltaTime);

	UPROPERTY(BlueprintAssignable, Category = "Upside Drown", meta = (DisplayName = "OnPushDelegate"))
		FOnPushPullSignature OnPushDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Upside Drown", meta = (DisplayName = "OnPullDelegate"))
		FOnPushPullSignature OnPullDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Upside Drown", meta = (DisplayName = "OnAttachDetachFireflyDelegate"))
		FAttachDelegateSignature OnAttachDetachDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Upside Drown", meta = (DisplayName = "OnTargetedDelegate"))
		FAttachDelegateSignature OnTargetUnTargetDelegate;
protected:
	UPROPERTY(SaveGame)
		bool bTargetActive;



	UPROPERTY(SaveGame)
		class UStickyLight* connectedLight;

};
