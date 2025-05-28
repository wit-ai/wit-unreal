/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "SWitUnderstandingViewerTab.h"
#include "Misc/EngineVersionComparison.h"
#include "DetailLayoutBuilder.h"
#include "Voice/Experience/VoiceExperience.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Engine/Selection.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Tool/Utilities/WitEditorHelperUtilities.h"

#define LOCTEXT_NAMESPACE "SWitUnderstandingViewerTab"

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
#if UE_VERSION_OLDER_THAN(5,0,0)
	Args.bShowActorLabel = false;
#else
	Args.bShowObjectLabel = false;
#endif
	Args.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	Args.ColumnWidth = 0.5f;

	DetailsWidget = PropertyModule.CreateDetailView(Args);
	DetailsWidget->SetVisibility(EVisibility::Hidden);

	ResponseObject = NewObject<UWitResponseObject>();
	ResponseObject->AddToRoot();
	
	ResponseObject->DetailsWidget = DetailsWidget;
	
	DetailsWidget->SetObject(ResponseObject, true);
	
	ChildSlot
	[
		SNew(SScrollBox)
		 
		+ SScrollBox::Slot().VAlign(VAlign_Top).Padding(10)
		[
			SNew(SVerticalBox)

			// Section to contain the text input and send button
			
			+ SVerticalBox::Slot().AutoHeight().Padding(5)
			[
				SNew(STextBlock)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity( FLinearColor( 0.5f, 0.5f, 0.5f, 1.0f ) )
				.Text(LOCTEXT("SendMessageTitle", "Send message"))
			]
			
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.Padding(5)
				.BorderImage( WitEditorHelperUtilities::GetBrush("ToolPanel.GroupBorder") )
				.Content()
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot().Padding(0, 0).AutoHeight()
					[
						SNew(SHorizontalBox)
						
						+ SHorizontalBox::Slot().VAlign(VAlign_Center).FillWidth(0.1f).Padding(10, 0)
						[
							SNew(STextBlock)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.ToolTipText(LOCTEXT("UtteranceTooltip", "The message to send to Wit.ai for intepretation"))
							.Text(LOCTEXT("UtteranceButton", "Utterance"))
						]

						+ SHorizontalBox::Slot().FillWidth(0.9f).Padding(0, 1, 10, 1)
						[
							SNew(SEditableTextBox)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.OnTextCommitted(this, &SWitUnderstandingViewerTab::OnUtteranceTextCommitted)
							.OnTextChanged(this, &SWitUnderstandingViewerTab::OnUtteranceTextChanged)
							.OnKeyDownHandler(this, &SWitUnderstandingViewerTab::OnUtteranceKeyDown)
						]
					]

					+ SVerticalBox::Slot().Padding(0, 0).AutoHeight()
					[
						SNew(SHorizontalBox)
			
						+ SHorizontalBox::Slot().HAlign(HAlign_Right).Padding(10,5,10,2)
						[
							SNew(SButton)
							.ToolTipText(LOCTEXT("SendTooltip", "Send the message to Wit.ai for interpretation"))
							.Text(LOCTEXT("SendButton", "Send"))
							.IsEnabled(this, &SWitUnderstandingViewerTab::IsSendButtonEnabled)
							.OnClicked(this, &SWitUnderstandingViewerTab::OnSendButtonClicked)
						]
					]
				]
			]	

			+ SVerticalBox::Slot().Padding(0,10)
			[
				SNew(SBox)
			]
			
			// Section to contain both the usage messaging and the actual response from Wit.ai
			
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SOverlay)

				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill).Padding(12.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UnderstandingViewerUsage1", "Select a Voice Experience actor to begin."))
					.Visibility(this, &SWitUnderstandingViewerTab::GetSelectMessageVisibility)
				]

				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill).Padding(12.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UnderstandingViewerUsage2", "Enter an utterance and hit 'Send' to see what your app will return."))
					.Visibility(this, &SWitUnderstandingViewerTab::GetUtteranceMessageVisibility)
				]

				+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Fill).Padding(12.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UnderstandingViewerUsage3", "Waiting for response..."))
					.Visibility(this, &SWitUnderstandingViewerTab::GetWaitMessageVisibility)
				]

				+ SOverlay::Slot()
				[
					SNew(SVerticalBox)
					.Visibility(this, &SWitUnderstandingViewerTab::GetResultVisibility)
										
					+ SVerticalBox::Slot().AutoHeight().Padding(5)
					[
						SNew(STextBlock)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
						.ColorAndOpacity( FLinearColor( 0.5f, 0.5f, 0.5f, 1.0f ) )
						.Text(LOCTEXT("ResultTitle", "Result"))
					]

					+ SVerticalBox::Slot().AutoHeight()
					[
						SNew(SBorder)
						.Padding(5)
						.BorderImage( WitEditorHelperUtilities::GetBrush("ToolPanel.GroupBorder") )
						.Content()
						[
							DetailsWidget.ToSharedRef()
						]
					]
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

	const AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();
	
	if (VoiceExperience == nullptr)
	{
		return EVisibility::Visible;	
	}
	
	return EVisibility::Hidden;
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
 * Whether we should show the result
 * 
 * @return true if we should show otherwise false
 */
EVisibility SWitUnderstandingViewerTab::GetResultVisibility() const
{
	if (GetSelectMessageVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}

	if (GetUtteranceMessageVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}

	if (GetWaitMessageVisibility() == EVisibility::Visible)
	{
		return EVisibility::Hidden;
	}

	return EVisibility::Visible;
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
	VoiceService->SetConfiguration(VoiceExperience->Configuration, false /* bUseWebSocket */);
	
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

	USelection* SelectedActors = GEditor->GetSelectedActors();

	for(FSelectionIterator It(*SelectedActors); It; ++It)
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


#undef LOCTEXT_NAMESPACE
