// Copyright © Farbspiel Interactive GmbH & Co. KG

#include "PrimaryGameLayout.h"

void UPrimaryGameLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	// We're not sure what layer the widget is on so go searching.
	for (const auto& LayerKVP : Layers)
	{
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonActivatableWidgetContainerBase* UPrimaryGameLayout::GetLayerWidget(FGameplayTag LayerName) const
{
	return Layers.FindRef(LayerName);
}

void UPrimaryGameLayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())
	{
		LayerWidget->SetTransitionDuration(0.0);
		LayerWidget->OnDisplayedWidgetChanged().AddWeakLambda(this, [this, LayerTag](UCommonActivatableWidget* Widget)
		{
			OnDisplayedWidgetChangedOnLayer.Broadcast(LayerTag, Widget);
		});

		Layers.Add(LayerTag, LayerWidget);
	}
}
