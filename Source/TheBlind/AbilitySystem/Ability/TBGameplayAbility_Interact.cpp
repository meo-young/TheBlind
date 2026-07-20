#include "TBGameplayAbility_Interact.h"
#include "TBGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/TBPlayerController.h"
#include "Interact/Interactable.h"

void UTBGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ATBPlayerController* PC = GetTBPlayerControllerFromActorInfo();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	
	FHitResult HitResult;
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);

	// 현재 마우스 커서 아래에 있는 오브젝트를 탐색합니다.
	if (PC->GetHitResultUnderCursorByChannel(TraceChannel,false, HitResult))
	{
		if (AActor* TargetActor = HitResult.GetActor())
		{
			if (IInteractable* Interactable = Cast<IInteractable>(TargetActor))
			{
				const float Distance = FVector::Dist2D(AvatarActor->GetActorLocation(), TargetActor->GetActorLocation());
				if (Distance <= InteractionRange)
				{
					// Interact() 호출 전에 완료 이벤트를 기다리는 Task를 등록합니다.
					// 시퀀스가 빠르게 종료돼도 이벤트를 놓치지 않기 위함입니다.
					UAbilityTask_WaitGameplayEvent* WaitFinishedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
							this,
							TBGameplayTags::GameplayEvent_Interact_Finished,
							nullptr,
							true,
							true);
					
					WaitFinishedTask->EventReceived.AddDynamic(this, &ThisClass::HandleInteractionFinished);
					WaitFinishedTask->ReadyForActivation();
					
					if (Interactable->Interact(*PC))
					{
						return;
					}
				}
			}
		}
	}
	
	K2_EndAbility();
	return;
}

void UTBGameplayAbility_Interact::HandleInteractionFinished(FGameplayEventData Payload)
{
	K2_EndAbility();
}
