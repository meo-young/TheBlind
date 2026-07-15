#include "TBInputConfig.h"

const UInputAction* UTBInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FTBInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	
	return nullptr;
}

const UInputAction* UTBInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FTBInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	
	return nullptr;
}