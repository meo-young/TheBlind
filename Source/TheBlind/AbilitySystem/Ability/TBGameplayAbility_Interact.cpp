#include "TBGameplayAbility_Interact.h"
#include "TBGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/TBPlayerController.h"
#include "Interact/Interactable.h"

void UTBGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ATBPlayerController* PC = GetTBPlayerControllerFromActorInfo();

	// 카메라 중앙의 대상이 현재 상호작용 가능한지 확인합니다.
	if (IInteractable* Interactable = PC->FindInteractionTarget())
	{
		if (Interactable->CanInteract())
		{
			// 카메라 전환이 빠르게 끝나도 완료 이벤트를 놓치지 않도록 먼저 대기 Task를 등록합니다.
			UAbilityTask_WaitGameplayEvent* WaitFinishedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TBGameplayTags::GameplayEvent_Interact_Finished, nullptr, true, true);
			WaitFinishedTask->EventReceived.AddDynamic(this, &ThisClass::HandleInteractionFinished);
			WaitFinishedTask->ReadyForActivation();

			// 상호작용이 시작되면 카메라 복귀 완료 시점까지 Ability를 유지합니다.
			if (Interactable->Interact(*PC))
			{
				return;
			}
		}
	}

	// 탐색 또는 상호작용 시작에 실패하면 Ability를 즉시 종료합니다.
	K2_EndAbility();
}

void UTBGameplayAbility_Interact::HandleInteractionFinished(FGameplayEventData Payload)
{
	K2_EndAbility();
}
