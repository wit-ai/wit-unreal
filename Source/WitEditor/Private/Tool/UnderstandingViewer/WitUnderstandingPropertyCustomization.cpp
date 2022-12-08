/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitUnderstandingPropertyCustomization.h"

#include "IDetailChildrenBuilder.h"
#include "SlateBasics.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Engine/Selection.h"
#include "Voice/Matcher/VoiceIntentMatcher.h"
#include "Voice/Matcher/VoiceIntentWithEntityMatcher.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Voice/Experience/VoiceExperience.h"
#include "Voice/Matcher/VoiceIntentWithEntitiesMatcher.h"

#define LOCTEXT_NAMESPACE "FWitUnderstandingPropertyCustomization"

/**
 * Convenience function to create an instance of the customization
 * 
 * @return the instance created
 */
TSharedRef<IPropertyTypeCustomization> FWitUnderstandingPropertyCustomization::MakeInstance()
{
	return MakeShareable(new FWitUnderstandingPropertyCustomization());
}

/**
 * Called to customize the header of the FWitIntent UStruct
 * 
 * @param StructPropertyHandle property handle for the structure
 * @param HeaderRow row widget
 * @param StructCustomizationUtils additional customization options 
 */
void FWitUnderstandingPropertyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// Retrieve the name property. We need this when creating an intent handler
	
	IntentNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWitIntent, Name));
	check(IntentNamePropertyHandle.IsValid());

	const TSharedPtr<IPropertyHandle> ResponsePropertyHandle = StructPropertyHandle->GetParentHandle()->GetParentHandle();
	if (ResponsePropertyHandle != nullptr)
	{
		EntitiesPropertyHandle = ResponsePropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWitResponse, Entities));
	}
	else
	{
		EntitiesPropertyHandle = nullptr;
	}
		
	const TSharedPtr<SWidget> ValueWidget = StructPropertyHandle->CreatePropertyValueWidget(false);
	
	HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			[
				ValueWidget.ToSharedRef()
			]
			
			+ SHorizontalBox::Slot().AutoWidth().Padding(2,6).VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("IntentAddHandlerTooltip", "Add a matcher for this intent to the selected voice experience"))
				.Text(LOCTEXT("IntentAddHandlerButton", "+ Intent Matcher"))
				.OnClicked(this, &FWitUnderstandingPropertyCustomization::OnAddIntentMatcherClicked)
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(2,6).VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("IntentWithEntityAddHandlerTooltip", "Add a matcher for this intent and a single entity to the selected voice experience"))
				.Text(LOCTEXT("IntentWithEntityAddHandlerButton", "+ Entity Matcher"))
				.OnClicked(this, &FWitUnderstandingPropertyCustomization::OnAddIntentWithEntityMatcherClicked)
			]

			+ SHorizontalBox::Slot().AutoWidth().Padding(2,6).VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("IntentWithEntitiesAddHandlerTooltip", "Add a matcher for this intent and all entities to the selected voice experience"))
				.Text(LOCTEXT("IntentWithEntitiesAddHandlerButton", "+ Entities Matcher"))
				.OnClicked(this, &FWitUnderstandingPropertyCustomization::OnAddIntentWithEntitiesMatcherClicked)
			]
		];
}

/**
 * Called to customize the children (members) of the FWitIntent UStruct
 * 
 * @param StructPropertyHandle property handle for the structure
 * @param StructBuilder build used for building the customizations
 * @param StructCustomizationUtils additional customization options
 */
void FWitUnderstandingPropertyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const TSharedPtr<IPropertyHandle> IdPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWitIntent, Id));
	check(IdPropertyHandle.IsValid());
	
	const TSharedPtr<IPropertyHandle> ConfidencePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWitIntent, Confidence));
	check(ConfidencePropertyHandle.IsValid());

	StructBuilder.AddProperty(IdPropertyHandle.ToSharedRef());
	StructBuilder.AddProperty(IntentNamePropertyHandle.ToSharedRef());
	StructBuilder.AddProperty(ConfidencePropertyHandle.ToSharedRef());
}

/**
 * Callback when the add intent matcher button is clicked
 */
FReply FWitUnderstandingPropertyCustomization::OnAddIntentMatcherClicked()
{
	UE_LOG(LogTemp, Display, TEXT("Adding intent handler"));

	if (!CanAddMatcher())
	{
		return FReply::Unhandled();
	}

	AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();

	if (VoiceExperience == nullptr)
	{
		return FReply::Unhandled();
	}
	
	UVoiceIntentMatcher* NewIntentMatcher = Cast<UVoiceIntentMatcher>(AddNewComponent(VoiceExperience, UVoiceIntentMatcher::StaticClass()));

	if (NewIntentMatcher == nullptr)
	{
		return FReply::Unhandled();
	}
	
	FString IntentNameAsString;
	IntentNamePropertyHandle->GetValue(IntentNameAsString);
		
	NewIntentMatcher->IntentName = IntentNameAsString;

	// Force a re-select of the object to get it to refresh

	GEditor->SelectNone(false,false);
	GEditor->SelectActor(VoiceExperience, true, true, false, true);
	GEditor->SelectComponent(NewIntentMatcher, true, true, false);

	return FReply::Handled();
}

/**
 * Callback when the add intent with entity button is clicked
 */
FReply FWitUnderstandingPropertyCustomization::OnAddIntentWithEntityMatcherClicked()
{
	UE_LOG(LogTemp, Display, TEXT("Adding intent with entity handler"));

	if (!CanAddMatcher())
	{
		return FReply::Unhandled();
	}

	AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();

	if (VoiceExperience == nullptr)
	{
		return FReply::Unhandled();
	}
	
	UVoiceIntentWithEntityMatcher* NewIntentWithEntityMatcher = Cast<UVoiceIntentWithEntityMatcher>(AddNewComponent(VoiceExperience, UVoiceIntentWithEntityMatcher::StaticClass()));

	if (NewIntentWithEntityMatcher == nullptr)
	{
		return FReply::Unhandled();
	}
	
	FString IntentNameAsString;
	IntentNamePropertyHandle->GetValue(IntentNameAsString);

	NewIntentWithEntityMatcher->IntentName = IntentNameAsString;
	
	// Retrieve the entity names and add them

	uint32 NumEntities = 0;

	if (EntitiesPropertyHandle != nullptr)
	{
		EntitiesPropertyHandle->GetNumChildren(NumEntities);
	}
	
	for (uint32 i = 0; i < NumEntities; ++i)
	{
		const TSharedPtr<IPropertyHandle> EntityValuePropertyHandle = EntitiesPropertyHandle->GetChildHandle(i);
		if (!EntityValuePropertyHandle.IsValid())
		{
			continue;
		}

		const TSharedPtr<IPropertyHandle> EntityKeyPropertyHandle = EntityValuePropertyHandle->GetKeyHandle();
		if (!EntityKeyPropertyHandle.IsValid())
		{
			continue;
		}
	
		FString EntityNameAsString;
		const FPropertyAccess::Result AccessResult = EntityKeyPropertyHandle->GetValue(EntityNameAsString);

		const bool bIsValidNameValue = ((AccessResult == FPropertyAccess::Success) && !EntityNameAsString.IsEmpty());
		if (!bIsValidNameValue)
		{
			continue;
		}

		NewIntentWithEntityMatcher->RequiredEntityName = EntityNameAsString;

		break;
	}

	// Force a re-select of the object to get it to refresh

	GEditor->SelectNone(false,false);
	GEditor->SelectActor(VoiceExperience, true, true, false, true);
	GEditor->SelectComponent(NewIntentWithEntityMatcher, true, true, false);

	return FReply::Handled();
}

/**
 * Callback when the add intent with entities button is clicked
 */
FReply FWitUnderstandingPropertyCustomization::OnAddIntentWithEntitiesMatcherClicked()
{
	UE_LOG(LogTemp, Display, TEXT("Adding intent with entities handler"));

	if (!CanAddMatcher())
	{
		return FReply::Unhandled();
	}

	AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();

	if (VoiceExperience == nullptr)
	{
		return FReply::Unhandled();
	}
	
	UVoiceIntentWithEntitiesMatcher* NewIntentWithEntitiesMatcher = Cast<UVoiceIntentWithEntitiesMatcher>(AddNewComponent(VoiceExperience, UVoiceIntentWithEntitiesMatcher::StaticClass()));

	if (NewIntentWithEntitiesMatcher == nullptr)
	{
		return FReply::Unhandled();
	}
	
	FString IntentNameAsString;
	IntentNamePropertyHandle->GetValue(IntentNameAsString);

	NewIntentWithEntitiesMatcher->IntentName = IntentNameAsString;
	
	// Retrieve the entity names and add them

	uint32 NumEntities = 0;

	if (EntitiesPropertyHandle != nullptr)
	{
		EntitiesPropertyHandle->GetNumChildren(NumEntities);
	}
	
	for (uint32 i = 0; i < NumEntities; ++i)
	{
		const TSharedPtr<IPropertyHandle> EntityValuePropertyHandle = EntitiesPropertyHandle->GetChildHandle(i);
		if (!EntityValuePropertyHandle.IsValid())
		{
			continue;
		}

		const TSharedPtr<IPropertyHandle> EntityKeyPropertyHandle = EntityValuePropertyHandle->GetKeyHandle();
		if (!EntityKeyPropertyHandle.IsValid())
		{
			continue;
		}
	
		FString EntityNameAsString;
		const FPropertyAccess::Result AccessResult = EntityKeyPropertyHandle->GetValue(EntityNameAsString);

		const bool bIsValidNameValue = ((AccessResult == FPropertyAccess::Success) && !EntityNameAsString.IsEmpty());
		if (!bIsValidNameValue)
		{
			continue;
		}

		NewIntentWithEntitiesMatcher->RequiredEntityNames.Add(EntityNameAsString);
	}

	// Force a re-select of the object to get it to refresh

	GEditor->SelectNone(false,false);
	GEditor->SelectActor(VoiceExperience, true, true, false, true);
	GEditor->SelectComponent(NewIntentWithEntitiesMatcher, true, true, false);

	return FReply::Handled();
}

/**
 * Are we allowed to add a matcher?
 */
bool FWitUnderstandingPropertyCustomization::CanAddMatcher() const
{
	if (!IntentNamePropertyHandle->IsValidHandle())
	{
		UE_LOG(LogTemp, Warning, TEXT("Name property is invalid"));
		return false;
	}
		
	FString ValueAsString;
	const FPropertyAccess::Result AccessResult = IntentNamePropertyHandle->GetValue(ValueAsString);

	const bool bIsValidNameValue = ((AccessResult == FPropertyAccess::Success) && !ValueAsString.IsEmpty());
	
	if (!bIsValidNameValue)
	{
		UE_LOG(LogTemp, Warning, TEXT("Name value could not be read from name property"));
		return false;
	}

	const AVoiceExperience* VoiceExperience = GetSelectedVoiceExperience();

	if (VoiceExperience == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("The selected object is not a voice experience"));
		return false;
	}

	const bool bIsWorld = VoiceExperience->GetWorld() != nullptr;
	
	if (!bIsWorld)
	{
		UE_LOG(LogTemp, Warning, TEXT("There is no world for the Wit API"));
		return false;
	}

	const bool bIsClassDefaultObject = VoiceExperience->HasAnyFlags(RF_ClassDefaultObject);
	
	if (bIsClassDefaultObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add matcher to class default object"));
		return false;
	}

	return true;
}

/**
 * Adds a new component of the given class to the given actor. This function is based on AddNewComponent is SCSSEditor.cpp. However it dose
 * not update the editor view correctly and therefore is less than ideal. I'm still not yet clear on the correct way to do this
 * 
 * @param ActorInstance [in] the actor to add the component to
 * @param NewComponentClass [in] the component class to add
 * @return the newly created and added component
 */
UActorComponent* FWitUnderstandingPropertyCustomization::AddNewComponent(AActor* ActorInstance, UClass* NewComponentClass) const
{
	// Begin a transaction. The transaction will end when the component name will be provided/confirmed by the user.
	
	TUniquePtr<FScopedTransaction> AddTransaction = MakeUnique<FScopedTransaction>( LOCTEXT("AddComponent", "Add Component") );

	ActorInstance->Modify();

	// Create an appropriate name for the new component
	
	const FName NewComponentName = *FComponentEditorUtils::GenerateValidVariableName(NewComponentClass, ActorInstance);
			
	// Get the set of owned components that exists prior to instancing the new component
	
	TInlineComponentArray<UActorComponent*> PreInstanceComponents;
	ActorInstance->GetComponents(PreInstanceComponents);

	// Construct the new component and attach as needed
	
	UActorComponent* NewInstanceComponent = NewObject<UActorComponent>(ActorInstance, NewComponentClass, NewComponentName, RF_Transactional);
	
	// Add to SerializedComponents array so it gets saved
	
	ActorInstance->AddInstanceComponent(NewInstanceComponent);
	NewInstanceComponent->OnComponentCreated();
	NewInstanceComponent->RegisterComponent();

	// Register any new components that may have been created during construction of the instanced component, but were not explicitly registered.
	
	TInlineComponentArray<UActorComponent*> PostInstanceComponents;
	ActorInstance->GetComponents(PostInstanceComponents);

	for (UActorComponent* ActorComponent : PostInstanceComponents)
	{
		if (!ActorComponent->IsRegistered() && ActorComponent->bAutoRegister && IsValid(ActorComponent) && !PreInstanceComponents.Contains(ActorComponent))
		{
			ActorComponent->RegisterComponent();
		}
	}

	// Rerun construction scripts
	
	ActorInstance->RerunConstructionScripts();

	return NewInstanceComponent;
}

/**
 * Gets the selected WitVoiceExperience in the scene (if any)
 * 
 * @return pointer to the Voice Experience actor if selected otherwise null
 */
AVoiceExperience* FWitUnderstandingPropertyCustomization::GetSelectedVoiceExperience()
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

#undef LOCTEXT_NAMESPACE
