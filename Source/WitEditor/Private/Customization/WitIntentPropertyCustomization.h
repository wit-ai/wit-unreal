/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "IPropertyTypeCustomization.h"
#include "Wit/Voice/WitVoiceService.h"

/**
 * Applies a property customization to the FWitIntent UStruct in order to add a context sensitive menu for
 * easily dding intent matchers
 */
class FWitIntentPropertyCustomization final : public IPropertyTypeCustomization
{
public:
	
	/**
	 * Convenience function to create an instance of the customization
	 * 
	 * @return the instance created
	 */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/**
	 * Called to customize the header of the FWitIntent UStruct
	 * 
	 * @param StructPropertyHandle [in] property handle for the structure
	 * @param HeaderRow [in] row widget
	 * @param StructCustomizationUtils [in] additional customization options 
	 */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	
	/**
	 * Called to customize the children (members) of the FWitIntent UStruct
	 * 
	 * @param StructPropertyHandle [in] property handle for the structure
	 * @param StructBuilder [in] builder used for building the customizations
	 * @param StructCustomizationUtils [in] additional customization options
	 */
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:

	/**
	 * Callback when the context sensitive menu option is selected to add an intent matcher
	 */
	void OnAddIntentMatcher() const;

	/**
	 * Callback when the context sensitive menu option is selected to add an intent with entity matcher
	 */
	void OnAddIntentWithEntityMatcher() const;

	/**
	 * Are we allowed to add a matcher?
	 */
	bool CanAddMatcher() const;
	
	/**
	 * Adds a new component of the given class to the given actor
	 * 
	 * @param ActorInstance [in] the actor to add the component to
	 * @param NewComponentClass [in] the component class to add
	 * @return the newly created and added component
	 */
	UActorComponent* AddNewComponent(AActor* ActorInstance, UClass* NewComponentClass) const;

private:

	/** Stores the outer objects that we are currently customizing */
	TArray<UObject*> OuterObjects{};

	/** Stores the property handle for the intent name member */
	TSharedPtr<IPropertyHandle> IntentNamePropertyHandle{};

	/** Stores the property handle for the entities map */
	TSharedPtr<IPropertyHandle> EntitiesPropertyHandle{};

};
