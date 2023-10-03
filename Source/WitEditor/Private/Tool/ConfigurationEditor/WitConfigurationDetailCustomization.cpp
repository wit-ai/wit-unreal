/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitConfigurationDetailCustomization.h"

#include "PropertyCustomizationHelpers.h"
#include "Input/Reply.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Wit/Configuration/WitAppConfiguration.h"
#include "Wit/Configuration/WitAppConfigurationAsset.h"
#include "Wit/Utilities/WitConfigurationUtilities.h"

#define LOCTEXT_NAMESPACE "FWitConfigurationDetailCustomization"

/**
 * Convenience function to create an instance of the customization
 * 
 * @return the instance created
 */
TSharedRef<IDetailCustomization> FWitConfigurationDetailCustomization::MakeInstance()
{
	return MakeShareable(new FWitConfigurationDetailCustomization);
}

/**
 * Called to customize the layout of the details panel for the Configuration class
 * 
 * @param DetailBuilder builder used for building/customizing the details panel
 */
void FWitConfigurationDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.GetObjectsBeingCustomized(ObjectsToEdit);
	
	// Change the token fields so they are password fields

	IDetailCategoryBuilder& TokenCategory = DetailBuilder.EditCategory("Token", FText::GetEmpty(), ECategoryPriority::Default);

	// Server token
	
	const TSharedRef<IPropertyHandle> ServerTokenProperty = DetailBuilder.GetProperty("Application.ServerAccessToken");
	
	if (!ServerTokenProperty->IsValidHandle())
	{
		return;
	}
	
	TSharedPtr<SWidget> NameWidget;
	TSharedPtr<SWidget> ValueWidget;
		
	IDetailPropertyRow& ServerTokenRow = TokenCategory.AddProperty(ServerTokenProperty);
	
	ServerTokenRow.GetDefaultWidgets(NameWidget,ValueWidget);

	MarkAsPassword(ValueWidget);
	
	ServerTokenRow.CustomWidget()
		.NameContent()
		[
			NameWidget.ToSharedRef()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			[
				ValueWidget.ToSharedRef()
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(6,0)
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshButton","Refresh"))
				.IsEnabled(this, &FWitConfigurationDetailCustomization::IsRefreshButtonEnabled)
				.OnClicked(this, &FWitConfigurationDetailCustomization::OnRefreshButtonClicked)
			]
		];

	// Client token
	
	const TSharedRef<IPropertyHandle> ClientTokenProperty = DetailBuilder.GetProperty("Application.ClientAccessToken");
	
	if (!ClientTokenProperty->IsValidHandle())
	{
		return;
	}
	
	IDetailPropertyRow& ClientTokenRow = TokenCategory.AddProperty(ClientTokenProperty);
	
	ClientTokenRow.GetDefaultWidgets(NameWidget,ValueWidget);

	MarkAsPassword(ValueWidget);
	
	ClientTokenRow.CustomWidget()
		.NameContent()
		[
			NameWidget.ToSharedRef()
		]
		.ValueContent()
		[
			ValueWidget.ToSharedRef()
		];	
}

/**
 * Callback when the refresh button is clicked
 * 
 * @return whether the reply was handled or not
 */
FReply FWitConfigurationDetailCustomization::OnRefreshButtonClicked()
{
	if (ObjectsToEdit.Num() == 0)
	{
		return FReply::Unhandled();
	}
	
	const TWeakObjectPtr<UObject> Object = ObjectsToEdit[0];
	
	if (!Object.IsValid())
	{
		return FReply::Unhandled();
	}

	UWitAppConfigurationAsset* AppConfiguration = Cast<UWitAppConfigurationAsset>(Object.Get());
		
	if (AppConfiguration == nullptr)
	{
		return FReply::Unhandled();
	}
	
	FWitConfigurationUtilities::RefreshConfiguration(AppConfiguration);
	
	return FReply::Handled();
}

/**
 * Determines if the refresh button should be enabled or not
 * 
 * @return true if enabled otherwise false
 */
bool FWitConfigurationDetailCustomization::IsRefreshButtonEnabled() const
{
	if (FWitConfigurationUtilities::IsRefreshInProgress())
	{
		return false;
	}
	
	if (ObjectsToEdit.Num() == 0)
	{
		return false;
	}
	
	const TWeakObjectPtr<UObject> Object = ObjectsToEdit[0];
	
	if (!Object.IsValid())
	{
		return false;
	}

	const UWitAppConfigurationAsset* AppConfiguration = Cast<UWitAppConfigurationAsset>(Object.Get());
		
	if (AppConfiguration == nullptr)
	{
		return false;
	}

	const bool bHasValidServerToken = !AppConfiguration->Application.ServerAccessToken.IsEmpty();

	return bHasValidServerToken;
}

/**
 * Marks the first child editable box as being a password
 */
void FWitConfigurationDetailCustomization::MarkAsPassword(TSharedPtr<SWidget> RootWidget) const
{
	static FName SEditableTextBoxType(TEXT("SEditableTextBox"));

	TArray<TSharedRef<SWidget>> Stack;
	Stack.Push(RootWidget.ToSharedRef());

	while (Stack.Num() > 0)
	{
		const TSharedRef<SWidget> Widget = Stack.Pop();

		if (Widget->GetType() == SEditableTextBoxType)
		{
			TSharedRef<SEditableTextBox> EditableTextBox = StaticCastSharedRef<SEditableTextBox, SWidget>(Widget);
			EditableTextBox->SetIsPassword(true);

			return;
		}
		
		FChildren* ChildWidgets = Widget->GetChildren();
		
		for (int32 i = 0; i < ChildWidgets->Num(); i++)
		{
			Stack.Push(ChildWidgets->GetChildAt(i));
		}
	}
}

#undef LOCTEXT_NAMESPACE
