/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "SWitUnderstandingViewerTab.h"
#include "Voice/Experience/VoiceExperience.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Selection.h"
#include "Editor.h"

/**
 * Construct the panel for the understanding viewer
 *
 * @param InArgs [in] the arguments associated with this tool
 */
void SWitUnderstandingViewerTab::Construct(const FArguments& InArgs)
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	FDetailsViewArgs Args;
	
	Args.bAllowSearch = false;
	Args.bAllowFavoriteSystem = false;
	Args.bHideSelectionTip = true;
	Args.bShowObjectLabel = false;
	Args.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	Args.ColumnWidth = 0.5f;

	DetailsWidget = PropertyModule.CreateDetailView(Args);
	DetailsWidget->SetVisibility(EVisibility::Hidden);

	ResponseObject = NewObject<UWitResponseObject>();
	ResponseObject->DetailsWidget = DetailsWidget;
	
	DetailsWidget->SetObject(ResponseObject, true);
	
	ChildSlot
	[
		SNew(SVerticalBox)
		 
		+ SVerticalBox::Slot().VAlign(VAlign_Top).FillHeight(1).Padding(10)
		[
			SNew(SVerticalBox)

			// Section to contain the text input and send button
			
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().VAlign(VAlign_Center).AutoWidth().Padding(10, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Utterance")))
				]

				+ SHorizontalBox::Slot().Padding(10, 1)
				[
					SNew(SEditableTextBox)
					.SelectAllTextWhenFocused(true)
					.OnTextCommitted_Raw(this, &SWitUnderstandingViewerTab::OnUtteranceTextCommitted)
					.OnTextChanged_Raw(this, &SWitUnderstandingViewerTab::OnUtteranceTextChanged)
					.OnKeyDownHandler_Raw(this, &SWitUnderstandingViewerTab::OnUtteranceKeyDown)
				]
					
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Send")))
					.IsEnabled_Raw(this, &SWitUnderstandingViewerTab::IsSendButtonEnabled)
					.OnClicked_Raw(this, &SWitUnderstandingViewerTab::OnSendButtonClicked)
				]
			]

			// Section to contain both the usage messaging and the actual response from Wit.ai
			
			+ SVerticalBox::Slot().Padding(0, 10)
			[
				SNew(SOverlay)

				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill).Padding(12.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Select a Voice Experience actor to begin.")))
					.Visibility(this, &SWitUnderstandingViewerTab::GetSelectMessageVisibility)
				]

				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill).Padding(12.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Enter an utterance and hit 'Send' to see what your app will return.")))
					.Visibility(this, &SWitUnderstandingViewerTab::GetUtteranceMessageVisibility)
				]

				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill).Padding(12.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Waiting for response...")))
					.Visibility(this, &SWitUnderstandingViewerTab::GetWaitMessageVisibility)
				]

				+ SOverlay::Slot()
				[
					DetailsWidget.ToSharedRef()
				]
			]
		]
	];
}

/**
 * Whether we should show the selection usage message
 * 
 * @return true if we should show otherwise false
 */
EVisibility SWitUnderstandingViewerTab::GetSelectMessageVisibility() const
{
	if (GetWaitMessageVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}
	
	if (DetailsWidget->GetVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}
	
	const AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();
	
	if (VoiceExperience != nullptr)
	{
		return EVisibility::Hidden;	
	}
	
	return EVisibility::Visible;
}

/**
 * Whether we should show the utterance usage message
 * 
 * @return true if we should show otherwise false
 */
EVisibility SWitUnderstandingViewerTab::GetUtteranceMessageVisibility() const
{
	if (GetWaitMessageVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}
	
	if (DetailsWidget->GetVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}

	const AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();
	
	if (VoiceExperience == nullptr)
	{
		return EVisibility::Hidden;	
	}
	
	return EVisibility::Visible;
}

/**
 * Whether we should show the waiting usage message
 * 
 * @return true if we should show otherwise false
 */
EVisibility SWitUnderstandingViewerTab::GetWaitMessageVisibility() const
{
	const AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();
	
	if (VoiceExperience == nullptr)
	{
		return EVisibility::Hidden;	
	}

	if (VoiceExperience->IsRequestInProgress())
	{
		return EVisibility::Visible;
	}
	
	return EVisibility::Hidden;
}

/**
 * Callback when the send button is clicked. This sends the utterance off to Wit.ai for processing
 * 
 * @return whether the reply was handled or not
 */
FReply SWitUnderstandingViewerTab::OnSendButtonClicked()
{
	const AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();
	
	if (VoiceExperience == nullptr || VoiceExperience->VoiceService == nullptr || VoiceExperience->VoiceEvents == nullptr)
	{
		return FReply::Handled();
	}
		
	if (VoiceExperience->IsRequestInProgress())
	{
		return FReply::Handled();
	}

	ResponseObject->Response.Reset();
	DetailsWidget->SetVisibility(EVisibility::Hidden);
	
	UVoiceService* VoiceService = VoiceExperience->VoiceService;

	VoiceService->SetEvents(VoiceExperience->VoiceEvents);
	VoiceService->SetConfiguration(VoiceExperience->Configuration);
	
	VoiceExperience->VoiceEvents->OnWitResponse.AddUniqueDynamic(ResponseObject, &UWitResponseObject::OnWitResponse);
	
	VoiceService->SendTranscription(UtteranceText.ToString());

	return FReply::Handled();
}

/**
 * Determines if the send button should be enabled or not
 * 
 * @return true if enabled otherwise false
 */
bool SWitUnderstandingViewerTab::IsSendButtonEnabled() const
{
	const AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();

	return VoiceExperience != nullptr;
}

/**
 * Callback when the utterance text box loses focus ("return" not able to trigger this )
 * 
 * @param InText [in] the text that was entered 
 */
void SWitUnderstandingViewerTab::OnUtteranceTextCommitted(const FText& InText, ETextCommit::Type)
{
	UtteranceText = InText;
}

/**
 * Callback when the utterance text is changed
 *
 * @param InText [in] the text that was entered
 */
void SWitUnderstandingViewerTab::OnUtteranceTextChanged(const FText& InText)
{
	UtteranceText = InText;
}

/**
 * Callback when the utterance text on focus and the user pressed down a keyboard key.
 *
 * This method will "send" the utterance if the key is "enter".
 *
 * @param KeyEvent [in] the key event, e.g. KeyDown.  This event also has the "key" -- KeyEvent.GetKey().
 */
FReply SWitUnderstandingViewerTab::OnUtteranceKeyDown(const FGeometry&, const FKeyEvent& KeyEvent)
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
 * Gets the selected WitVoiceExperience in the scene (if any)
 * 
 * @return pointer to the Voice Experience actor if selected otherwise null
 */
AVoiceExperience* SWitUnderstandingViewerTab::GetSelectedVoiceExperience()
{
	const bool bIsSingleSelectedActor = GEditor->GetSelectedActorCount() == 1;
	
	if (!bIsSingleSelectedActor)
	{
		return nullptr;
	}

	for (FSelectionIterator It = GEditor->GetSelectedActorIterator(); It; ++It)
	{
		return Cast<AVoiceExperience>(*It);
	}

	return nullptr;
}

/**
 * Callback that is called when a Wit.ai response is received. Checks to see if the response matches what
 * we are looking for
 * 
 * @param bIsSuccessful [in] true if the response was successful
 * @param WitResponse [in] the full response as a UStruct
 */
void UWitResponseObject::OnWitResponse(const bool bIsSuccessful, const FWitResponse& WitResponse)
{
	if (!bIsSuccessful)
	{
		return;
	}

	Response = WitResponse;
	DetailsWidget->SetVisibility(EVisibility::Visible);
}
