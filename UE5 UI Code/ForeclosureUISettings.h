// Copyright © Farbspiel Interactive GmbH & Co. KG

#pragma once

#include "CoreMinimal.h"
#include "PrimaryGameLayout.h"
#include "UIStyle.h"
#include "ForeclosureUISettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogForeclosureUI, Display, All);

/**
 * Settings class that manages central UI settings. Currently not user facing 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Project UI Settings"))
class FORECLOSURE_API UForeclosureUISettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, BlueprintCallable, meta = (DisplayName="Get Foreclosure UI Settings"))
	static UForeclosureUISettings* GetUISettings();

	UFUNCTION(BlueprintPure, BlueprintCallable)
	static UUIStyle* RequestDefaultUIStyle();
	
	UFUNCTION(BlueprintPure, BlueprintCallable)
	class UUIStyle* GetDefaultUIStyle() const;

	float GetButtonHighlightDuration() const;

	UFUNCTION(BlueprintPure, BlueprintCallable)
	TSubclassOf<UPrimaryGameLayout> GetPrimaryGameLayout() const;
	
protected:
	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override;
	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const override;

	UPROPERTY(EditDefaultsOnly, Config, meta=(AllowedClasses=UIStyle))
	FSoftObjectPath DefaultUIStyle;

	//Class that is used by the HUD to manage different menu/modal screens
	UPROPERTY(EditDefaultsOnly, Config, meta=(AllowedClasses=PrimaryGameLayout))
	FSoftClassPath PrimaryGameLayoutClass;

	//The duration for which a button should light up when pressed by a button
	UPROPERTY(EditDefaultsOnly, Config)
	float ButtonHighlightDuration = 0.5f;
};
