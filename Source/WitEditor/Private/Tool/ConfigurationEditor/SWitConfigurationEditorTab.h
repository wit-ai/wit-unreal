﻿/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Wit/Configuration/WitAppConfigurationAsset.h"
#include "SWitConfigurationEditorTab.generated.h"

class AVoiceExperience;

/**
 * UObject representation of the sdk settings
 */
UCLASS(BlueprintType)
class UWitEditedConfiguration final : public UObject
{
	GENERATED_BODY()

public:

	/** The underlying response structure */
	UPROPERTY(Transient, EditAnywhere, Category = "Selected")
	UWitAppConfigurationAsset* Configuration{};
	
	/** The details widget that will display this UObject */
	TSharedPtr<IDetailsView> DetailsWidget{};

	/** The details widget that will display this UObject */
	TSharedPtr<IDetailsView> DetailsContentWidget{};

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		if (DetailsContentWidget != nullptr)
		{
			DetailsContentWidget->SetObject(Configuration);
		}
	}	
};

/**
 * Slate widget to represent the Settings tab. The settings tab is used to create and configure wit app settings
 */
class SWitConfigurationEditorTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SWitConfigurationEditorTab) {}
	SLATE_END_ARGS()

	/** Define the slate layout for the widget */
	void Construct(const FArguments& InArgs);

protected:

	/** Callback used to determine visibility of the create preset */
	EVisibility GetCreatePresetVisibility() const;

	/** Callback when the create preset button is clicked */
	FReply OnCreatePresetButtonClicked();

	/** Callback when the new button is clicked */
	FReply OnNewButtonClicked();

	/** Callback to determine if the new button should be enabled */
	bool IsNewButtonEnabled() const;

	/** Callback when the server token text is changed */
	void OnServerTokenTextChanged(const FText& InText);

	/** Callback when the configuration name text is changed */
	void OnNewConfigurationTextChanged(const FText& InText);

	/** Create a new configuration asset */
	UWitAppConfigurationAsset* CreateConfigurationAsset();
	
	/** Details widget that will display the settings object */
	TSharedPtr<IDetailsView> DetailsWidget{};

	/** Details widget that will display the settings object */
	TSharedPtr<IDetailsView> DetailsContentWidget{};

	/** A UObject wrapper for the settings structure so that we can display it in the details widget */
	UWitEditedConfiguration* EditedConfiguration{};

	/** The currently entered token text */
	FText ServerTokenText{};

	/** The currently entered new configuration name */
	FText NewConfigurationText{};
};
