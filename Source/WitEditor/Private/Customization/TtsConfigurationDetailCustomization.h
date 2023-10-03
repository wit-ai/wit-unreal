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
 * Apples a details customization to the 'Settings' category of the UWitTtsService class. This adds an additional
 * 'Understanding' section that allows us to enter text queries and send them to Wit.ai in order to see the response
 */
class FTtsConfigurationDetailCustomization final : public IDetailCustomization
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

	/**
	 * Callback when the utterance text on focus and the user pressed down a keyboard key.
	 *
	 * This method will "send" the utterance if the key is "enter".
	 *
	 * @param KeyEvent [in] the key event, e.g. KeyDown.  This event also has the "key" -- KeyEvent.GetKey().
	 */
	FReply OnEnterKey(const FGeometry& Geo, const FKeyEvent& KeyEvent);

	/**
	 * Callback when the utterance text is changed
	 *
	 * @param InText [in] the text that was entered
	 */
	void OnUtteranceTextChanged(const FText& InText);

private:

	/**
	 * Callback when the send button is clicked. This sends the utterance off to Wit.ai for processing
	 * 
	 * @return whether the reply was handled or not
	 */
	FReply OnSendButtonClicked();
	
	/**
     * Callback when the delete button is clicked
     * 
     * @return whether the reply was handled or not
     */
    FReply OnDeleteButtonClicked();

	/**
	 * Callback when the fetch available voices button is clicked
	 * 
	 * @return whether the reply was handled or not
	 */
	FReply OnFetchVoicesButtonClicked();

	/**
	 * Callback when the create preset button is clicked
	 * 
	 * @return whether the reply was handled or not
	 */
	FReply OnCreatePresetButtonClicked();

	/**
	 * Callback when synthesize response returns
	 */
	void OnSynthesizeResponse(const bool bIsSuccessful, USoundWave* SoundWave);
	
	/**
	 * Callback when the utterance text is confirmed with a return
	 * 
	 * @param InText [in] the text that was entered 
	 */
	void OnUtteranceTextCommitted(const FText& InText, ETextCommit::Type);

private:

	/** The objects we are currently editing */
	TArray<TWeakObjectPtr<UObject>> ObjectsToEdit{};

	/** The currently entered utterance */
	FText UtteranceText{};

};
