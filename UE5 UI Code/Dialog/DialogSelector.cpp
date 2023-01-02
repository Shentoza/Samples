// Copyright © Farbspiel Interactive GmbH & Co. KG

#include "DialogSelector.h"

#include "DialogOption.h"
#include "DlgContext.h"
#include "Components/PanelWidget.h"
#include "Foreclosure/Core/PlayerControllerIngame.h"
#include "Foreclosure/Dialogue/DialogueSubsystem.h"
#include "Foreclosure/Utils/ForeclosureAccessors.h"

void UDialogSelector::NativeConstruct()
{
	Super::NativeConstruct();
	auto DialogSubsystem = GetWorld()->GetSubsystem<UDialogueSubsystem>();
	if (IsValid(DialogSubsystem))
	{
		DialogSubsystem->OnDialogueUpdated.AddUniqueDynamic(this, &UDialogSelector::HandleDialogueUpdate);
	}
	for (const auto DialogOption : DialogOptionPanel->GetAllChildren())
	{
		if (auto Casted = Cast<UDialogOption>(DialogOption))
		{
			DialogOptionWidgets.AddUnique(Casted);
		}
	}

	auto PC = UForeclosureAccessors::GetPlayerControllerIngame(GetWorld());
	if (IsValid(PC))
	{
		PC->OnChosenDialogueOptionChanged.AddUniqueDynamic(this, &UDialogSelector::HandleChosenDialogOptionChanged);
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UDialogSelector::HandleDialogueUpdate(UDlgContext* NewContext, UObject* Initiator)
{
	if (!IsValid(NewContext))
	{
		SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const auto OptionsNum = NewContext->GetOptionsNum();
	int CurrentChosenIndex = 0;
	if (auto PC = UForeclosureAccessors::GetPlayerControllerIngame(GetWorld()))
	{
		CurrentChosenIndex = PC->GetChosenDialogOptionIndex();
	}
	if (OptionsNum > 1)
	{
		SetVisibility(ESlateVisibility::Visible);
		for (int i = 0; i < DialogOptionWidgets.Num(); ++i)
		{
			if (NewContext->IsValidOptionIndex(i))
			{
				DialogOptionWidgets[i]->SetDialogOption(NewContext->GetOptionText(i),
				                                        NewContext->GetOption(i).EdgeData);
				DialogOptionWidgets[i]->SetSelected(CurrentChosenIndex == i);
				DialogOptionWidgets[i]->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				DialogOptionWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}

void UDialogSelector::HandleChosenDialogOptionChanged(int ChosenIndex)
{
	if (auto DialogSubsystem = GetWorld()->GetSubsystem<UDialogueSubsystem>())
	{
		if (DialogSubsystem->IsDialogOptionPreselected())
		{
			return;
		}
	}

	for (int i = 0; i < DialogOptionWidgets.Num(); ++i)
	{
		if (IsValid(DialogOptionWidgets[i]))
		{
			DialogOptionWidgets[i]->SetSelected(ChosenIndex == i);
		}
	}
}
