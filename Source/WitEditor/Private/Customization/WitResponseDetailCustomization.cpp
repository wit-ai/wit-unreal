/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitResponseDetailCustomization.h"
#include "Voice/Experience/VoiceExperience.h"

#include "Input/Reply.h"

#include "PropertyEditor/Public/DetailLayoutBuilder.h"
#include "PropertyEditor/Public/DetailCategoryBuilder.h"
#include "PropertyEditor/Public/DetailWidgetRow.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FWitEditorModule"

/**
 * Convenience function to create an instance of the customization
 * 
 * @return the instance created
 */
TSharedRef<IDetailCustomization> FWitResponseDetailCustomization::MakeInstance()
{
	return MakeShareable(new FWitResponseDetailCustomization);
}

/**
 * Called to customize the layout of the details panel for the UWitVoiceService class
 * 
 * @param DetailBuilder builder used for building/customizing the details panel
 */
void FWitResponseDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.GetObjectsBeingCustomized(ObjectsToEdit);

	// Add a text entry and button so that we can easily send message requests to the Wit API to see the response
	
	IDetailCategoryBuilder& ResponseCategory = DetailBuilder.EditCategory("Experience", FText::GetEmpty());

    ResponseCategory.AddCustomRow(LOCTEXT("Keyword", "Utterance"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NameText", "Utterance"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SEditableTextBox)
			.SelectAllTextWhenFocused(true)
			.OnTextCommitted_Raw(this, &FWitResponseDetailCustomization::OnUtteranceTextCommitted)
			.OnTextChanged_Raw(this, &FWitResponseDetailCustomization::OnUtteranceTextChanged)
			.OnKeyDownHandler_Raw(this, &FWitResponseDetailCustomization::OnEnterKey)
		];
		
	ResponseCategory.AddCustomRow(LOCTEXT("Keyword", "Send"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("ButtonText", "Send"))
			.HAlign(HAlign_Center)
			.IsEnabled_Raw(this, &FWitResponseDetailCustomization::IsSendButtonEnabled)
			.OnClicked_Raw(this, &FWitResponseDetailCustomization::OnSendButtonClicked)
		];
}

/**
 * Callback when the utterance text on focus and the user pressed down a keyboard key.
 *
 * This method will "send" the utterance if the key is "enter".
 *
 * @param KeyEvent [in] the key event, e.g. KeyDown.  This event also has the "key" -- KeyEvent.GetKey().
 */
FReply FWitResponseDetailCustomization::OnEnterKey(const FGeometry&, const FKeyEvent& KeyEvent)
{
	if (KeyEvent.GetKey() == EKeys::Enter)
	{
		return OnSendButtonClicked();
	}
	else
	{
		return FReply::Unhandled();
	}
}

/**
 * Determines if the send button should be enabled or not
 * 
 * @return true if enabled otherwise false
 */
bool FWitResponseDetailCustomization::IsSendButtonEnabled() const
{
	// Always allow sending in case the user wants to send an empty string for some reason
	
	return true;
}

/**
 * Callback when the send button is clicked. This sends the utterance off to Wit.ai for processing
 * 
 * @return whether the reply was handled or not
 */
FReply FWitResponseDetailCustomization::OnSendButtonClicked()
{
	UE_LOG(LogTemp, Display, TEXT("Sending message request %d"), ObjectsToEdit.Num());
	
	for (TWeakObjectPtr<UObject> Object : ObjectsToEdit)
	{
		if (!Object.IsValid())
		{
			continue;
		}

		// Send off a message request to Wit.ai to get the response. The response can then be used to help setup
		// response matchers easily

		const UVoiceEvents* VoiceEvents = Cast<UVoiceEvents>(Object.Get());

		if (VoiceEvents == nullptr)
		{
			continue;
		}

		const AVoiceExperience* VoiceExperience = Cast<AVoiceExperience>(VoiceEvents->GetOwner());

		if (VoiceExperience == nullptr || VoiceExperience->VoiceService == nullptr)
		{
			continue;
		}
		
		if (VoiceExperience->IsRequestInProgress())
		{
			continue;
		}

		UVoiceService* VoiceService = VoiceExperience->VoiceService;

		VoiceService->SetEvents(VoiceExperience->VoiceEvents);
		VoiceService->SetConfiguration(VoiceExperience->Configuration);
		VoiceService->SendTranscription(UtteranceText.ToString());

		// We can only handle one request at a time even if multi-select is used so exit here
		
		break;
	}

	return FReply::Handled();
}

/**
 * Callback when the utterance text box loses focus ("return" not able to trigger this )
 * 
 * @param InText [in] the text that was entered 
 */
void FWitResponseDetailCustomization::OnUtteranceTextCommitted(const FText& InText, ETextCommit::Type)
{
	UtteranceText = InText;
}

/**
 * Callback when the utterance text is changed
 *
 * @param InText [in] the text that was entered
 */
void FWitResponseDetailCustomization::OnUtteranceTextChanged(const FText& InText)
{
	UtteranceText = InText;
}

#undef LOCTEXT_NAMESPACE
