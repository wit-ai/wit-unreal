/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TtsConfigurationDetailCustomization.h"
#include "Input/Reply.h"
#include "PropertyEditor/Public/DetailLayoutBuilder.h"
#include "PropertyEditor/Public/DetailCategoryBuilder.h"
#include "PropertyEditor/Public/DetailWidgetRow.h"
#include "TTS/Experience/TtsExperience.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FWitEditorModule"

/**
 * Convenience function to create an instance of the customization
 * 
 * @return the instance created
 */
TSharedRef<IDetailCustomization> FTtsConfigurationDetailCustomization::MakeInstance()
{
	return MakeShareable(new FTtsConfigurationDetailCustomization);
}

/**
 * Called to customize the layout of the details panel for the UWitTtsService class
 * 
 * @param DetailBuilder builder used for building/customizing the details panel
 */
void FTtsConfigurationDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.GetObjectsBeingCustomized(ObjectsToEdit);
	
	// Add a text entry and button so that we can easily send message requests to the Wit API to see the response. Disabled for now until play sound works

	IDetailCategoryBuilder& ConfigurationCategory = DetailBuilder.EditCategory("TTS", FText::GetEmpty(), ECategoryPriority::Important);

#if defined(WIT_ENABLE_SYNTHESIZE_IN_EDITOR)
	ConfigurationCategory.AddCustomRow(LOCTEXT("Keyword", "Send"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("SendButtonText", "Send"))
			.HAlign(HAlign_Center)
			.OnClicked_Raw(this, &FTtsConfigurationDetailCustomization::OnSendButtonClicked)
		];
#endif
	
	// Add a text entry and button so that we can easily send message requests to the Wit API to see the response
	
	IDetailCategoryBuilder& CacheCategory = DetailBuilder.EditCategory("TTS Cache", FText::GetEmpty(), ECategoryPriority::Important);

	CacheCategory.AddCustomRow(LOCTEXT("Keyword", "Delete"))
	.ValueContent()
	[
		SNew(SButton)
		.Text(LOCTEXT("DeleteButtonText", "Delete Cached Files"))
		.HAlign(HAlign_Center)
		.OnClicked_Raw(this, &FTtsConfigurationDetailCustomization::OnDeleteButtonClicked)
	];
}

/**
 * Callback when the utterance text on focus and the user pressed down a keyboard key.
 *
 * This method will "send" the utterance if the key is "enter".
 *
 * @param KeyEvent [in] the key event, e.g. KeyDown.  This event also has the "key" -- KeyEvent.GetKey().
 */
FReply FTtsConfigurationDetailCustomization::OnEnterKey(const FGeometry&, const FKeyEvent& KeyEvent)
{
	if (KeyEvent.GetKey() == EKeys::Enter)
	{
		return OnSendButtonClicked();
	}

	return FReply::Unhandled();
}

/**
 * Callback when the send button is clicked. This sends the utterance off to Wit.ai for processing
 * 
 * @return whether the reply was handled or not
 */
FReply FTtsConfigurationDetailCustomization::OnSendButtonClicked()
{
	UE_LOG(LogTemp, Display, TEXT("Covert text request"));
	
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid())
		{
			continue;
		}

		// Send off a message request to Wit.ai to get the response. The response can then be used to help setup
		// response matchers easily

		ATtsExperience* TtsExperience = Cast<ATtsExperience>(Object.Get());
		
		if (TtsExperience == nullptr)
		{
			continue;
		}

		if (TtsExperience->IsRequestInProgress())
		{
			continue;
		}

        // At the moment all this will do is put the clip into the caches. To be useful it needs to be able to play the 
        // sound so the user can hear it. Need to investigate how to do this
        
		TtsExperience->ConvertTextToSpeechWithSettings(TtsExperience->VoicePreset->Synthesize);
		
		break;
	}

	return FReply::Handled();
}

/**
 * Callback when the delete button is clicked
 * 
 * @return whether the reply was handled or not
 */
FReply FTtsConfigurationDetailCustomization::OnDeleteButtonClicked()
{
	UE_LOG(LogTemp, Display, TEXT("Delete cached files"));

	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid())
		{
			continue;
		}
		
		ATtsExperience* TtsExperience = Cast<ATtsExperience>(Object.Get());

		if (TtsExperience == nullptr)
		{
			continue;
		}

		// This only deletes files not assets. Deleting assets from the content folder should be left to the user in the content browser
		// as it's potentially destructive
		
		TtsExperience->DeleteAllClips(ETtsStorageCacheLocation::Persistent);
		TtsExperience->DeleteAllClips(ETtsStorageCacheLocation::Temporary);
		
		break;
	}

	return FReply::Handled();
}

/**
 * Callback when the utterance text box loses focus ("return" not able to trigger this )
 * 
 * @param InText [in] the text that was entered 
 */
void FTtsConfigurationDetailCustomization::OnUtteranceTextCommitted(const FText& InText, ETextCommit::Type)
{
	UtteranceText = InText;
}

/**
 * Callback when the utterance text is changed
 *
 * @param InText [in] the text that was entered
 */
void FTtsConfigurationDetailCustomization::OnUtteranceTextChanged(const FText& InText)
{
	UtteranceText = InText;
}

#undef LOCTEXT_NAMESPACE
