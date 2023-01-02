// Copyright © Farbspiel Interactive GmbH & Co. KG

#pragma once

#include "CoreMinimal.h"
#include "Foreclosure/UI/Common/ForeclosureUserWidget.h"
#include "DialogOption.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDialogOption, Log, All);

/**
 * Widget representing a singular dialog option that can be selected.
 * Shows the text and has handling for highlighting it.
 */
UCLASS()
class FORECLOSURE_API UDialogOption : public UForeclosureUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintSetter)
	void SetSelected(bool bNewIsSelected);

	void SetDialogOption(FText Text, class UDlgNodeData* Data = nullptr);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveOnSelected(bool bNewIsSelected);

	virtual void NativeConstruct() override;

	virtual void ApplyUIStyle() override;

	virtual void HandleSelected(bool bNewIsSelected);

private:

	UPROPERTY(BlueprintSetter = SetSelected)
	bool bIsSelected = false;
	
	UPROPERTY(meta = (BindWidget))
	class UForeclosureTextBlock* DialogText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = true))
	class UForeclosureImage* BackgroundImage;
};
