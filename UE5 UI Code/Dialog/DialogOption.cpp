// Copyright © Farbspiel Interactive GmbH & Co. KG

#include "DialogOption.h"

#include "Foreclosure/Dialogue/DialogueBlacklistAction.h"
#include "Foreclosure/UI/ForeclosureUISettings.h"
#include "Foreclosure/UI/Common/ForeclosureImage.h"
#include "Foreclosure/UI/Common/ForeclosureTextBlock.h"

#define LOCTEXT_NAMESPACE "UMG"


void UDialogOption::SetSelected(bool bNewIsSelected)
{
	if (bNewIsSelected != bIsSelected)
	{
		bIsSelected = bNewIsSelected;
		ReceiveOnSelected(bIsSelected);
		HandleSelected(bIsSelected);
	}
}

void UDialogOption::SetDialogOption(FText Text, UDlgNodeData* Data)
{
	if (auto BlacklistAction = Cast<UDialogueBlacklistAction>(Data))
	{
		Text = FText::Format(LOCTEXT("DialogOptionText", "{0} [Blacklist]"), Text);
	}
	DialogText->SetText(Text);
}

void UDialogOption::NativeConstruct()
{
	Super::NativeConstruct();
	ReceiveOnSelected(bIsSelected);
	HandleSelected(bIsSelected);
}

void UDialogOption::ApplyUIStyle()
{
	Super::ApplyUIStyle();
	const auto UIStyle = UForeclosureUISettings::RequestDefaultUIStyle();
	if (IsValid(UIStyle))
	{
		DialogText->TryApplyFontFromStyle(UIStyle->GetFont());
		DialogText->TryApplyColorAndOpacityFromStyle(UIStyle->GetFontColor());
		if (IsValid(BackgroundImage))
		{
			BackgroundImage->TryApplyTintColorFromStyle(UIStyle->GetForegroundWithTextColor());
		}
	}
	else
	{
		UE_LOG(LogForeclosureUI, Warning, TEXT("UI Style hasn't been defined"));
	}
}

void UDialogOption::HandleSelected(bool bNewIsSelected)
{
	auto UIStyle = UForeclosureUISettings::RequestDefaultUIStyle();
	auto FontColor = bNewIsSelected ? UIStyle->GetFontColor() : UIStyle->GetFontColorDisabled();
	auto BackgroundTint = bNewIsSelected ? UIStyle->GetForegroundWithTextColor() : UIStyle->GetDisabledColor();
	BackgroundImage->Brush.TintColor = BackgroundTint;
	DialogText->SetColorAndOpacity(FontColor);
}

#undef LOCTEXT_NAMESPACE
