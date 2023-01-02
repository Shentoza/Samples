// Copyright © Farbspiel Interactive GmbH & Co. KG

#include "HUDIngame.h"

#include "NativeGameplayTags.h"
#include "Foreclosure/UI/Common/ForeclosureUserWidget.h"
#include "Foreclosure/Core/FirstPersonCharacter.h"
#include "Foreclosure/Core/PlayerControllerIngame.h"
#include "Foreclosure/UI/ForeclosureUISettings.h"
#include "Foreclosure/UI/PlayerModeScreen.h"
#include "Foreclosure/UI/Dialog/DialogSelector.h"
#include "Foreclosure/Utils/ForeclosureAccessors.h"
#include "Components/InputComponent.h"

DEFINE_LOG_CATEGORY(LogForeclosureHUD);

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_MODAL, "UI.Layer.Modal");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_LAYER_GAME, "UI.Layer.Game");

UCommonActivatableWidget* AHUDIngame::AddWidgetToLayout(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
{
	if (!IsValid(GameLayout))
	{
		return nullptr;
	}
	auto NewWidget = GameLayout->PushWidgetToLayerStack<UCommonActivatableWidget>(
		LayerName, ActivatableWidgetClass, [](auto& Widget){});
	return NewWidget;
}

void AHUDIngame::RemoveWidgetFromLayout(UCommonActivatableWidget* WidgetToRemove)
{
	if (!IsValid(GameLayout))
	{
		return;
	}
	GameLayout->FindAndRemoveWidgetFromLayer(WidgetToRemove);
}

void AHUDIngame::ShowMainMenu()
{
	MainMenu = AddWidgetToLayout(TAG_UI_LAYER_MODAL, MainMenuClass);
}

void AHUDIngame::CloseTopMenu()
{
	if (auto Layer = GameLayout->GetLayerWidget(TAG_UI_LAYER_MODAL))
	{
		if (auto ActiveWidget = Layer->GetActiveWidget())
		{
			if(auto ForeclosureWidget = Cast<UForeclosureActivatableWidget>(ActiveWidget))
			{
				ForeclosureWidget->TryToDeactivate();	
			}
			else
			{
				ActiveWidget->DeactivateWidget();
			}
		}
	}
}

void AHUDIngame:: HandleDisplayedWidgetChangedOnLayer(const FGameplayTag Layer, UCommonActivatableWidget* Widget)
{
	if (const auto LayerWidget = GameLayout->GetLayerWidget(Layer))
	{
		// todo: Currently the only layer we track this way are modals for tutorial screens and exit prompts
		// Therefore we can assume the "Modal layer changed" = "need to broadcast MenuOpenenedState"
		// with more layers being used we have to differentiate here
		if (Layer == TAG_UI_LAYER_MODAL)
		{
			if (LayerWidget->GetNumWidgets() == 1)
			{
				OnMenuOpenedStateChanged.Broadcast(true);
			}
			else if (LayerWidget->GetNumWidgets() == 0)
			{
				OnMenuOpenedStateChanged.Broadcast(false);
			}
		}
	}
}

void AHUDIngame::EscapeMenu()
{
	if (auto Layer = GameLayout->GetLayerWidget(TAG_UI_LAYER_MODAL))
	{
		if (!IsValid(Layer->GetActiveWidget()))
		{
			ShowMainMenu();
		}
		else
		{
			CloseTopMenu();
		}
	}
}

void AHUDIngame::InitializeGameLayout()
{
	EnableInput(GetOwningPlayerController());
	if (IsValid(InputComponent))
	{
		InputComponent->BindAction("PauseMenu", IE_Released, this, &ThisClass::EscapeMenu);
	}
	auto LayoutClass = UForeclosureUISettings::GetUISettings()
		                   ? UForeclosureUISettings::GetUISettings()->GetPrimaryGameLayout()
		                   : UPrimaryGameLayout::StaticClass();
	GameLayout = CreateWidget<UPrimaryGameLayout>(GetWorld(), LayoutClass);
	if (IsValid(GameLayout))
	{
		//ZOrder 2, so we make sure other widgets (that are not properly handled in our now intended way don't interfere
		//with our UI layout
		GameLayout->AddToPlayerScreen(2);
		GameLayout->OnDisplayedWidgetChangedOnLayer.AddUniqueDynamic(
			this, &ThisClass::HandleDisplayedWidgetChangedOnLayer);
	}
}

void AHUDIngame::BeginPlay()
{
	Super::BeginPlay();

	InitializeGameLayout();
	InitializeBindings();
	InitializeDialogWidgets();
	InitializePlayerModeWidgets();
}

void AHUDIngame::InitializeDialogWidgets()
{
	auto PC = GetOwningPlayerController();

	if (IsValid(DialogSelectorWidgetClass))
	{
		DialogSelectorWidget = CreateWidget<UDialogSelector>(PC, DialogSelectorWidgetClass);
		DialogSelectorWidget->AddToViewport(0);
	}
	else
	{
		UE_LOG(LogForeclosureHUD, Warning, TEXT("No Dialog Selector Widget class set!"));
	}
}

void AHUDIngame::InitializeBindings()
{
	APlayerControllerIngame* PCIngame = UForeclosureAccessors::GetPlayerControllerIngame(GetWorld());
	if (auto Character = UForeclosureAccessors::GetPlayerPawn(GetWorld()))
	{
		Character->OnPlayerModeChanged.AddUniqueDynamic(this, &AHUDIngame::ReceivePlayerModeChanged);
	}
	else
	{
		GetOwningPlayerController()->GetOnNewPawnNotifier().AddLambda([this](APawn* NewPawn)
		{
			if (auto Character = Cast<AFirstPersonCharacter>(NewPawn))
			{
				Character->OnPlayerModeChanged.AddUniqueDynamic(this, &AHUDIngame::ReceivePlayerModeChanged);
			}
		});
	}
}

void AHUDIngame::InitializePlayerModeWidgets()
{
	EPlayerMode CurrentMode = EPlayerMode::Normal;
	if (auto Character = UForeclosureAccessors::GetPlayerPawn(GetWorld()))
	{
		CurrentMode = Character->GetPlayerMode();
	}
	if (const auto CurrentModeWidget = PlayerModeWidgets.Find(CurrentMode))
	{
		if (IsValid(CurrentModeWidget->WidgetClass))
		{
			ReceivePlayerModeChanged(EPlayerMode::MAX, CurrentMode);
		}
	}
}

void AHUDIngame::ReceivePlayerModeChanged(EPlayerMode OldPlayerMode, EPlayerMode NewPlayerMode)
{
	if (PlayerModeWidgets.Contains(OldPlayerMode))
	{
		const auto OldWidget = GameLayout->GetLayerWidget(TAG_UI_LAYER_GAME)->GetActiveWidget();
		if (IsValid(OldWidget))
		{
			OldWidget->DeactivateWidget();
		}
	}
	if (PlayerModeWidgets.Contains(NewPlayerMode) && IsValid(PlayerModeWidgets[NewPlayerMode].WidgetClass))
	{
		const auto NextWidget = AddWidgetToLayout(TAG_UI_LAYER_GAME, PlayerModeWidgets[NewPlayerMode].WidgetClass);
	}
}
