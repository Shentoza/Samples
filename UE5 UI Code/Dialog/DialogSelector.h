#pragma once

#include "CoreMinimal.h"
#include "Foreclosure/UI/Common/ForeclosureActivatableWidget.h"
#include "DialogSelector.generated.h"

/*
 * Widget class that holds the different dialog options and manages the representation of selecting different dialog options.
 */
UCLASS()
class FORECLOSURE_API UDialogSelector : public UForeclosureActivatableWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

protected:
	UFUNCTION()
	void HandleDialogueUpdate(class UDlgContext* NewContext, UObject* Initiator);

	UFUNCTION()
	void HandleChosenDialogOptionChanged(int ChosenIndex);
private:

	UPROPERTY(meta = (BindWidget))
	class UPanelWidget* DialogOptionPanel;

	UPROPERTY()
	TArray<class UDialogOption*> DialogOptionWidgets;
};