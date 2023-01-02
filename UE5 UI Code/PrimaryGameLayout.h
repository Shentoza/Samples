// Copyright © Farbspiel Interactive GmbH & Co. KG

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "Common/ForeclosureCommonUserWidget.h"
#include "UObject/Object.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "PrimaryGameLayout.generated.h"

/**
 * This is a class that holds different UI screens and items, to allow for stuff like menus opening over each other.
 * For now this is just a holder class, that is used by the HUDIngame class, but will later be extended upon
 * Takes inspiration from PrimaryGameLayout.h from the CommonGame plugin located in the Lyra Game example
 */
UCLASS()
class FORECLOSURE_API UPrimaryGameLayout : public UForeclosureCommonUserWidget
{
	GENERATED_BODY()
public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDisplayedWidgetChangedOnLayer, const FGameplayTag, Layer, UCommonActivatableWidget*, DisplayedWidget);
	
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass,
	                                           TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		checkf(ActivatableWidgetClass->IsChildOf(UCommonActivatableWidget::StaticClass()),TEXT("Only CommonActivatableWidgets can be used here"));

		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
		{
			auto AddedWidget = Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
			return AddedWidget;
		}

		return nullptr;
	}

	// Find the widget if it exists on any of the layers and remove it from the layer.
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	// Get the layer widget for the given layer tag.
	UCommonActivatableWidgetContainerBase* GetLayerWidget(FGameplayTag LayerName) const;

	UPROPERTY(BlueprintAssignable, DisplayName = "Displayed Widget Changed on Layer")
	FOnDisplayedWidgetChangedOnLayer OnDisplayedWidgetChangedOnLayer;

protected:
	/** Register a layer that widgets can be pushed onto. */
	UFUNCTION(BlueprintCallable, Category="Layer")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag,
	                   UCommonActivatableWidgetContainerBase* LayerWidget);


private:

	// The registered layers for the primary layout.
	UPROPERTY(Transient, meta = (Categories = "UI.Layer"))
	TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*> Layers;
};
