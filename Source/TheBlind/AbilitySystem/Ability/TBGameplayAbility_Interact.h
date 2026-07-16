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
	
	
	
// ─────────────────────────────────────────────────────────────
// Interaction
// ─────────────────────────────────────────────────────────────
protected:
	/** 플레이어가 오브젝트와 상호작용할 수 있는 최대 거리입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|상호작용")
	float InteractionRange = 100.0f;
	
};
