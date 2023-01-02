// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Abilities/StickyAbility/StickyTarget.h"
#include "HeatingTarget.generated.h"

//TODO: Macht es sinn nur für diese Schwellwerte zu broadcasten, oder möchte man das vielleicht immer, und der empfänger entscheidet welche nachrichten wichtig sind?
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTemperatureThresholdPassedSignature, const bool, isActive);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UPSIDEDROWN_API UHeatingTarget : public UStickyTarget
{
	GENERATED_BODY()


public:
	UHeatingTarget();

	virtual void OnPush(const float DeltaTime) override;

	virtual void OnPull(const float DeltaTime) override;

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY(BlueprintAssignable, Category = "Upside Drown")
		FTemperatureThresholdPassedSignature OnHiTempThresholdPassedDelegate;
	UPROPERTY(BlueprintAssignable, Category = "Upside Drown")
		FTemperatureThresholdPassedSignature OnLoTempThresholdPassedDelegate;


	UFUNCTION(BlueprintImplementableEvent, Category = "Upside Drown")
		void OnHiTempThresholdPassed(const bool bThresholdActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "Upside Drown")
		void OnLoTempThresholdPassed(const bool bThresholdActive);
protected:

	//TODO: default abkühlen/auftauen anders? PHYSIKALISCH MAYBE? bzw ein langsames kontinuierliches angleichen

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Settings")
		float fMinTemp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Settings")
		float fMaxTemp = 100.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Settings")
		float fTempPerSecond = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Settings")
		float fRevertingPerSecond = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Settings")
		float fRevertingCooldown = 5.0f;
	float fElapsedTime;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite, Category = "Temperature Settings")
		/*Current Heat: Wert zwischen 0..100.f gibt an wie erhitzt etwas ist.*/
		float fCurrentTemp = 50.f;

	float fOldTemp = fCurrentTemp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Settings")
		float fDefaultTemp = fCurrentTemp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Settings")
		float fHighTempThreshold = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature Settings")
		float fLoTempThreshold = 25.0f;

private:
	void CheckThresholds();
};
