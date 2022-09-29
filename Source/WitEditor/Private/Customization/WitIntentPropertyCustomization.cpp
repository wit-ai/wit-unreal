/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "WitIntentPropertyCustomization.h"

#include "IDetailChildrenBuilder.h"
#include "SlateBasics.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Voice/Matcher/VoiceIntentMatcher.h"
#include "Voice/Matcher/VoiceIntentWithEntityMatcher.h"
#include "Kismet2/ComponentEditorUtils.h"

#define LOCTEXT_NAMESPACE "FWitEditorModule"
#define ENABLE_INTENT_MATCHER_ACTION 1

/**
 * Convenience function to create an instance of the customization
 * 
 * @return the instance created
 */
TSharedRef<IPropertyTypeCustomization> FWitIntentPropertyCustomization::MakeInstance()
{
	return MakeShareable(new FWitIntentPropertyCustomization());
}

/**
 * Called to customize the header of the FWitIntent UStruct
 * 
 * @param StructPropertyHandle property handle for the structure
 * @param HeaderRow row widget
 * @param StructCustomizationUtils additional customization options 
 */
void FWitIntentPropertyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// Retrieve the outer objects. We need this to create the handlers
	
	StructPropertyHandle->GetOuterObjects(OuterObjects);

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
	
	const FUIAction AddIntentHandlerAction(FExecuteAction::CreateSP(this, &FWitIntentPropertyCustomization::OnAddIntentMatcher));
	const FUIAction AddIntentWithEntityHandlerAction(FExecuteAction::CreateSP(this, &FWitIntentPropertyCustomization::OnAddIntentWithEntityMatcher));

	HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			StructPropertyHandle->CreatePropertyValueWidget(false)
		];

#if ENABLE_INTENT_MATCHER_ACTION
	// Only add the right click context menu in scene view and not in the blueprint editor. Otherwise the world does not exist
	// and we get issues

	const bool bIsNoOuterObjects = OuterObjects.Num() == 0;
	
	if (bIsNoOuterObjects)
	{
		return;
	}
	
	const bool bIsWorld = OuterObjects[0]->GetWorld() != nullptr;
	if (bIsWorld)
	{
		HeaderRow
			.AddCustomContextMenuAction(AddIntentHandlerAction,
				LOCTEXT("IntentAddHandler", "Add Intent Matcher"),
				LOCTEXT("IntentAddHandlerTooltip", "Add a matcher for the selected intent"),
				FSlateIcon())
			.AddCustomContextMenuAction(AddIntentWithEntityHandlerAction,
				LOCTEXT("IntentWithEntityAddHandler", "Add Intent With Entity Matcher"),
				LOCTEXT("IntentWithEntityAddHandlerTooltip", "Add a matcher for the selected intent and entity"),
				FSlateIcon());
	}
#endif
}

/**
 * Called to customize the children (members) of the FWitIntent UStruct
 * 
 * @param StructPropertyHandle property handle for the structure
 * @param StructBuilder build used for building the customizations
 * @param StructCustomizationUtils additional customization options
 */
void FWitIntentPropertyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
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
 * Callback when the context sensitive menu option is selected to add an intent matcher
 */
void FWitIntentPropertyCustomization::OnAddIntentMatcher() const
{
	UE_LOG(LogTemp, Display, TEXT("Adding intent handler"));

	if (!CanAddMatcher())
	{
		return;
	}

	const UVoiceService* VoiceService = Cast<UVoiceService>(OuterObjects[0]);	
	UVoiceIntentMatcher* NewIntentMatcher = Cast<UVoiceIntentMatcher>(AddNewComponent(VoiceService->GetOwner(), UVoiceIntentMatcher::StaticClass()));

	const bool bIsNewIntentMatcher = NewIntentMatcher != nullptr;
	if (bIsNewIntentMatcher)
	{
		FString IntentNameAsString;
		IntentNamePropertyHandle->GetValue(IntentNameAsString);
		
		NewIntentMatcher->IntentName = IntentNameAsString;
	}

	// Force a re-select of the object to get it to refresh

	GEditor->SelectNone(false,false);
	GEditor->SelectActor(VoiceService->GetOwner(), true, true, false, true);
	GEditor->SelectComponent(NewIntentMatcher, true, true, false);
}

/**
 * Callback when the context sensitive menu option is selected to add an intent with entity matcher
 */
void FWitIntentPropertyCustomization::OnAddIntentWithEntityMatcher() const
{
	UE_LOG(LogTemp, Display, TEXT("Adding intent with entity handler"));

	if (!CanAddMatcher())
	{
		return;
	}

	const UVoiceService* VoiceService = Cast<UVoiceService>(OuterObjects[0]);	
	UVoiceIntentWithEntityMatcher* NewIntentWithEntityMatcher = Cast<UVoiceIntentWithEntityMatcher>(AddNewComponent(VoiceService->GetOwner(), UVoiceIntentWithEntityMatcher::StaticClass()));

	const bool bIsNewIntentMatcher = NewIntentWithEntityMatcher != nullptr;
	if (bIsNewIntentMatcher)
	{
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
		}
	}

	// Force a re-select of the object to get it to refresh

	GEditor->SelectNone(false,false);
	GEditor->SelectActor(VoiceService->GetOwner(), true, true, false, true);
	GEditor->SelectComponent(NewIntentWithEntityMatcher, true, true, false);
}

/**
 * Are we allowed to add a matcher?
 */
bool FWitIntentPropertyCustomization::CanAddMatcher() const
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
		
	const bool bHasSingleOuterObject = (OuterObjects.Num() == 1);
	if (!bHasSingleOuterObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("There are too many or too few outer objects"));
		return false;
	}

	const UVoiceService* VoiceService = Cast<UVoiceService>(OuterObjects[0]);

	const bool bIsWitObject = (VoiceService != nullptr);
	if (!bIsWitObject)
	{
		UE_LOG(LogTemp, Warning, TEXT("The outer object is not the Wit API"));
		return false;
	}

	const AActor* Actor = VoiceService->GetOwner();
		
	const bool bIsOwningActor = Actor != nullptr;
	if (!bIsOwningActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("There is no actor for the Wit API"));
		return false;
	}

	const bool bIsWorld = Actor->GetWorld() != nullptr;
	if (!bIsWorld)
	{
		UE_LOG(LogTemp, Warning, TEXT("There is no world for the Wit API"));
		return false;
	}

	const bool bIsClassDefaultObject = Actor->HasAnyFlags(RF_ClassDefaultObject);
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
UActorComponent* FWitIntentPropertyCustomization::AddNewComponent(AActor* ActorInstance, UClass* NewComponentClass) const
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

#undef LOCTEXT_NAMESPACE
