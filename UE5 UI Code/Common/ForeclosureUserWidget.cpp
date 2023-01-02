#include "ForeclosureUserWidget.h"

void UForeclosureUserWidget::SetVisibilityWithFeedback(ESlateVisibility InVisibility, bool bWithFeedback)
{
	const auto OldVisibility = GetVisibility();
	SetVisibility(InVisibility);
	OnVisibilityChangedWithFeedback.Broadcast(OldVisibility, InVisibility, bWithFeedback);
	ReceiveVisibilityChanged(OldVisibility, InVisibility, bWithFeedback);
}

void UForeclosureUserWidget::ApplyUIStyle()
{
	ReceiveApplyUIStyle();
}

void UForeclosureUserWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	ApplyUIStyle();
}
