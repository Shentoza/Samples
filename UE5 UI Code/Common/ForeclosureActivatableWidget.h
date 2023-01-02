// Copyright © Farbspiel Interactive GmbH & Co. KG

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Input/CommonInputMode.h"
#include "ForeclosureActivatableWidget.generated.h"

/**
 * 
 */
UCLASS()
class FORECLOSURE_API UForeclosureActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
public:
	//Tries to deactivate a widget, which can be prevented for things like menus that need to be open for a certain time
	void TryToDeactivate();
protected:
	
	UFUNCTION(BlueprintPure, BlueprintCallable, BlueprintNativeEvent)
	bool CanBeDeactivated() const;
private:
	UPROPERTY(EditDefaultsOnly, Category = "Input", BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	ECommonInputMode DesiredInputMode = ECommonInputMode::Game;

	UPROPERTY(EditDefaultsOnly, Category = "Input", BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	EMouseCaptureMode DesiredMouseCaptureMode = EMouseCaptureMode::CaptureDuringMouseDown;

	UPROPERTY(EditDefaultsOnly, Category = "Input", BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool DesiredHideCursorDuringViewportCapture = false;
};
