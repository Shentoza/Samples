// Copyright © Farbspiel Interactive GmbH & Co. KG

#include "ForeclosureActivatableWidget.h"

TOptional<FUIInputConfig> UForeclosureActivatableWidget::GetDesiredInputConfig() const
{
	const FUIInputConfig Config{DesiredInputMode, DesiredMouseCaptureMode, DesiredHideCursorDuringViewportCapture};
	return TOptional(Config);
}

void UForeclosureActivatableWidget::TryToDeactivate()
{
	if(CanBeDeactivated())
	{
		DeactivateWidget();
	}
}

bool UForeclosureActivatableWidget::CanBeDeactivated_Implementation() const
{
	return true;
}
