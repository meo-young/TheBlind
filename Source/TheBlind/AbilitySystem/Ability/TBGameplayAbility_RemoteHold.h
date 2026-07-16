#pragma once

#include "CoreMinimal.h"
#include "TBGameplayAbility.h"
#include "TBGameplayAbility_RemoteHold.generated.h"

UCLASS()
class THEBLIND_API UTBGameplayAbility_RemoteHold : public UTBGameplayAbility
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	
	
// ─────────────────────────────────────────────────────────────
// State
// ─────────────────────────────────────────────────────────────
private:
	/** Controller가 Remote Hold 시작 요청을 수락했는지 나타냅니다. */
	bool bRemoteHoldStarted = false;
	
};
