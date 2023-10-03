/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FReply;

/**
 * Applies a details customization to configuration structure
 */
class FWitConfigurationDetailCustomization final : public IDetailCustomization
{
public:

	/**
	 * Convenience function to create an instance of the customization
	 * 
	 * @return the instance created
	 */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/**
	 * Called to customize the layout of the details panel for the UWitVoiceService class
	 * 
	 * @param DetailBuilder builder used for building/customizing the details panel
	 */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:

	/** Callback when the refresh button is clicked */
	FReply OnRefreshButtonClicked();

	/** Callback to determine if the refresh button should be enabled */
	bool IsRefreshButtonEnabled() const;

	/** Mark a child editable text box as being a password */
	void MarkAsPassword(TSharedPtr<SWidget> RootWidget) const;

	/** The objects we are currently editing */
	TArray<TWeakObjectPtr<UObject>> ObjectsToEdit{};

};
