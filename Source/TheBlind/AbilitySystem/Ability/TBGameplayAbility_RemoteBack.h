#pragma once

#include "CoreMinimal.h"
#include "TBGameplayAbility.h"
#include "TBGameplayAbility_RemoteBack.generated.h"

UCLASS()
class THEBLIND_API UTBGameplayAbility_RemoteBack : public UTBGameplayAbility
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	
};
