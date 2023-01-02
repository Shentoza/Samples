// Copyright © Farbspiel Interactive GmbH & Co. KG

#pragma once

#include "CoreMinimal.h"
#include "Foreclosure/Core/PlayerModes.h"
#include "Foreclosure/UI/PrimaryGameLayout.h"
#include "GameFramework/HUD.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "HUDIngame.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogForeclosureHUD, Log, All);

USTRUCT(BlueprintType)
struct FORECLOSURE_API FPlayerModeWidgetUISettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UPlayerModeScreen> WidgetClass;

	UPROPERTY()
	class UPlayerModeScreen* Widget;

	UPROPERTY(EditDefaultsOnly)
	bool bClosePreviousWithFeedback = true;

	UPROPERTY(EditDefaultsOnly)
	bool bOpenNextWithFeedback = true;
};

/**
 * Base HUD class for ingame usage. Holds references to menu screens and widgets used on screen.
 */
UCLASS(Abstract)
class FORECLOSURE_API AHUDIngame : public AHUD
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenusOpenedChangedSignature, bool, bWidgetOpened);

	UPROPERTY(BlueprintAssignable, DisplayName= "Menu State Changed")
	FOnMenusOpenedChangedSignature OnMenuOpenedStateChanged;

	UFUNCTION(BlueprintCallable)
	UCommonActivatableWidget* AddWidgetToLayout(FGameplayTag LayerName, UClass* ActivatableWidgetClass);

	void RemoveWidgetFromLayout(UCommonActivatableWidget* WidgetToRemove);

protected:
	void InitializeGameLayout();

	virtual void BeginPlay() override;

	void InitializeDialogWidgets();

	void InitializeBindings();

	UFUNCTION()
	void InitializePlayerModeWidgets();

	UFUNCTION()
	void ReceivePlayerModeChanged(EPlayerMode OldPlayerMode, EPlayerMode NewPlayerMode);

	UFUNCTION(BlueprintCallable)
	void ShowMainMenu();

	void CloseTopMenu();

	UFUNCTION()
	void HandleDisplayedWidgetChangedOnLayer(const FGameplayTag Layer, UCommonActivatableWidget* Widget);

	void EscapeMenu();

private:
	UPROPERTY()
	UPrimaryGameLayout* GameLayout;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Used Widgets", AllowPrivateAccess=true))
	TSubclassOf<class UUserWidget> MainMenuClass;

	UPROPERTY()
	class UCommonActivatableWidget* MainMenu;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Used Widgets", AllowPrivateAccess=true))
	TSubclassOf<class UForeclosureActivatableWidget> DialogSelectorWidgetClass;

	UPROPERTY()
	class UForeclosureActivatableWidget* DialogSelectorWidget;

	UPROPERTY(EditDefaultsOnly, meta = (Category="Used Widgets", AllowPrivateAccess=true))
	TMap<EPlayerMode, FPlayerModeWidgetUISettings> PlayerModeWidgets;
};
