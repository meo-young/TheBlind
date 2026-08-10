#include "TBAnimInstance.h"
#include "Character/TBCharacter.h"

void UTBAnimInstance::OnInit_Implementation()
{
	Super::OnInit_Implementation();
	
	if (AActor* Owner = GetOwningActor())
	{
		if (ATBCharacter* IrisCharacter = Cast<ATBCharacter>(Owner))
		{
			InitializeWithAbilitySystem(IrisCharacter->GetAbilitySystemComponent());
		}
	}
}

void UTBAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	GameplayTagPropertyMap.Initialize(this, ASC);
}
