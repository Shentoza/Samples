// Copyright © Farbspiel Interactive GmbH & Co. KG

#include "ForeclosureUISettings.h"

DEFINE_LOG_CATEGORY(LogForeclosureUI);

UForeclosureUISettings* UForeclosureUISettings::GetUISettings()
{
	return GetMutableDefault<UForeclosureUISettings>();
}

UUIStyle* UForeclosureUISettings::RequestDefaultUIStyle()
{
	const auto Settings = GetUISettings();
	if(IsValid(Settings))
	{
		return Settings->GetDefaultUIStyle();
	}
	return nullptr;
}

UUIStyle* UForeclosureUISettings::GetDefaultUIStyle() const
{
	return Cast<UUIStyle>(DefaultUIStyle.TryLoad());
}

FName UForeclosureUISettings::GetContainerName() const
{
	return TEXT("Project");
}

FName UForeclosureUISettings::GetCategoryName() const
{
	return TEXT("Foreclosure");
}

float UForeclosureUISettings::GetButtonHighlightDuration() const
{
	return ButtonHighlightDuration;
}

TSubclassOf<UPrimaryGameLayout> UForeclosureUISettings::GetPrimaryGameLayout() const
{
	return PrimaryGameLayoutClass.TryLoadClass<UPrimaryGameLayout>();
}
