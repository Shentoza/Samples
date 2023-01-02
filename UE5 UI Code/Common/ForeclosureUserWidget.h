// Copyright © Farbspiel Interactive GmbH & Co. KG

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForeclosureUserWidget.generated.h"

/**
 * This class serves as an extensible baseclass. This way we can share common functionalities across all kind of
 * user widgets in this project, without having to touch/adjust engine code, or change multiple assets at once.
 */
UCLASS(Abstract, editinlinenew, BlueprintType, Blueprintable, meta = ( DontUseGenericSpawnObject="True", DisableNativeTick))
class FORECLOSURE_API UForeclosureUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnVisibilityChangedSignature, ESlateVisibility, OldVisibility,
	                                               ESlateVisibility, NewVisibility, bool, bWithFeedback);

	UPROPERTY(BlueprintAssignable)
	FOnVisibilityChangedSignature OnVisibilityChangedWithFeedback;

	//Override the native SetVisibility function, to enable a "OnVisibilityChanged" event
	virtual void SetVisibilityWithFeedback(ESlateVisibility InVisibility, bool bWithFeedback);

protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName ="Visibility Changed"))
	void ReceiveVisibilityChanged(ESlateVisibility OldVisibility, ESlateVisibility NewVisibility, bool bWithFeedback);

	UFUNCTION(BlueprintCallable)
	virtual void ApplyUIStyle();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName="Apply UI Style"))
	void ReceiveApplyUIStyle();

public:
	virtual void SynchronizeProperties() override;
};
