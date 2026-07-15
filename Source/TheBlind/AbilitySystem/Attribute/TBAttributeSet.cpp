#include "TBAttributeSet.h"
#include "AbilitySystem/TBAbilitySystemComponent.h"

void UTBAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

void UTBAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ClampAttribute(Attribute, NewValue);
}

UTBAbilitySystemComponent* UTBAttributeSet::GetAbilitySystemComponent() const
{
	return Cast<UTBAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
