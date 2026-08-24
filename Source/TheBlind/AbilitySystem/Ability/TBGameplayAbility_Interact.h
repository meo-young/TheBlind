#pragma once

#include "CoreMinimal.h"
#include "TBGameplayAbility.h"
#include "TBGameplayAbility_Interact.generated.h"

UCLASS()
class THEBLIND_API UTBGameplayAbility_Interact : public UTBGameplayAbility
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// GameplayAbility Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	
	
// ─────────────────────────────────────────────────────────────
// Interaction Callback
// ─────────────────────────────────────────────────────────────
private:
	/** 플레이어 시점으로 완전히 복귀했을 때 Ability를 종료합니다. */
	UFUNCTION()
	void HandleInteractionFinished(FGameplayEventData Payload);
	
};
