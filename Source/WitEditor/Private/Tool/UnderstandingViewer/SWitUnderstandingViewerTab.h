/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Wit/Request/WitResponse.h"
#include "SWitUnderstandingViewerTab.generated.h"

class AVoiceExperience;

/**
 * UObject representation of the full JSON object used by Wit.ai responses. See the Wit.ai
 * documentation for the meaning of each specific field.
 */
UCLASS(BlueprintType)
class UWitResponseObject final : public UObject
{
	GENERATED_BODY()

public:

	/** The response received from Wit.ai */
	UPROPERTY(Transient, EditAnywhere, Category = "Voice Experience")
	FWitResponse Response{};
	
	/** The details widget that will display this response */
	TSharedPtr<IDetailsView> DetailsWidget{};

	/** Callback to receive and update the response structure when Wit.ai responds */
	UFUNCTION()
	void OnWitResponse(const bool bIsSuccessful, const FWitResponse& WitResponse);

};

/**
 * Slate widget to represent the Understanding viewer. The Understanding viewer is a simple tool to be able to send
 * requests to Wit.ai and see the results in the editor
 */
class SWitUnderstandingViewerTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SWitUnderstandingViewerTab) {}
	SLATE_END_ARGS()

	/** Define the slate layout for the widget */
	void Construct(const FArguments& InArgs);

protected:

	/** Gets the currently selected voice experience or null if there is none */ 
	static AVoiceExperience* GetSelectedVoiceExperience();

	/** Callbacks used by the editable text box of the utterance text to track when the input text changes */
	void OnUtteranceTextChanged(const FText& InText);
	void OnUtteranceTextCommitted(const FText& InText, ETextCommit::Type);
	FReply OnUtteranceKeyDown(const FGeometry& Geometry, const FKeyEvent& KeyEvent);

	/** Callbacks used by the send button */
	FReply OnSendButtonClicked();
	bool IsSendButtonEnabled() const;

	/** Callbacks used to determine visibility of the tool usage messages */
	EVisibility GetSelectMessageVisibility() const;
	EVisibility GetUtteranceMessageVisibility() const;
	EVisibility GetWaitMessageVisibility() const;
	EVisibility GetResultVisibility() const;

	/** The current text entered by the user as an utterance */
	FText UtteranceText{};

	/** Details widget that will display the response that gets returned from Wit.ai */
	TSharedPtr<IDetailsView> DetailsWidget{};

	/** A UObject wrapper for the response structure so that we can display it in the details widget */
	UWitResponseObject* ResponseObject{};
	
};
