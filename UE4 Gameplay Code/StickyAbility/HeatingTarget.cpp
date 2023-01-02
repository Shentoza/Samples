// Fill out your copyright notice in the Description page of Project Settings.

#include "HeatingTarget.h"
#include "Components/SceneComponent.h"




UHeatingTarget::UHeatingTarget()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHeatingTarget::BeginPlay()
{
	Super::BeginPlay();
	fOldTemp = fCurrentTemp;
}

void UHeatingTarget::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	fElapsedTime = FMath::Clamp(fElapsedTime + DeltaTime, 0.0f, fRevertingCooldown);

	//Lange Zeit keine Aktion: Reverte zurück auf Ausgangstemperatur
	if (fElapsedTime == fRevertingCooldown) {
		if (fCurrentTemp > fDefaultTemp) {
			fCurrentTemp = FMath::Clamp(fCurrentTemp - DeltaTime * fRevertingPerSecond, fDefaultTemp, fMaxTemp);
		}
		else if(fCurrentTemp < fDefaultTemp) {
			fCurrentTemp = FMath::Clamp(fCurrentTemp + DeltaTime * fRevertingPerSecond, fMinTemp, fDefaultTemp);
		}
	}
	CheckThresholds();
}

void UHeatingTarget::OnPush(const float DeltaTime)
{	
	Super::OnPush(DeltaTime);

	fCurrentTemp = FMath::Clamp(fCurrentTemp + DeltaTime * fTempPerSecond, fMinTemp, fMaxTemp);
	fElapsedTime = 0.0f;

	CheckThresholds();
}

void UHeatingTarget::OnPull(const float DeltaTime)
{
	Super::OnPull(DeltaTime);

	fCurrentTemp = FMath::Clamp(fCurrentTemp - DeltaTime * fTempPerSecond, fMinTemp, fMaxTemp);
	fElapsedTime = 0.0f;
	CheckThresholds();
}

void UHeatingTarget::CheckThresholds()
{
	//High Temperature Check
	if (fOldTemp < fHighTempThreshold && fCurrentTemp >= fHighTempThreshold) {
		if(OnHiTempThresholdPassedDelegate.IsBound())
			OnHiTempThresholdPassedDelegate.Broadcast(true);

		OnHiTempThresholdPassed(true);
	}
	if (fOldTemp >= fHighTempThreshold && fCurrentTemp < fHighTempThreshold) {
		if (OnHiTempThresholdPassedDelegate.IsBound())
			OnHiTempThresholdPassedDelegate.Broadcast(false);

		OnHiTempThresholdPassed(false);
	}


	//Low Temperature Check
	if (fOldTemp > fLoTempThreshold && fCurrentTemp <= fLoTempThreshold) {
		if(OnLoTempThresholdPassedDelegate.IsBound())
			OnLoTempThresholdPassedDelegate.Broadcast(true);

		OnLoTempThresholdPassed(true);
	}
	if (fOldTemp <= fLoTempThreshold && fCurrentTemp > fLoTempThreshold) {
		if (OnLoTempThresholdPassedDelegate.IsBound())
			OnLoTempThresholdPassedDelegate.Broadcast(false);

		OnLoTempThresholdPassed(false);
	}

	fOldTemp = fCurrentTemp;
}



