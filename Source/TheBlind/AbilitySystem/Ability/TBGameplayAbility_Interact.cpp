#include "TBGameplayAbility_Interact.h"
#include "TBGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/Player/TBPlayerController.h"
#include "Interact/Interactable.h"

void UTBGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ATBPlayerController* PC = GetTBPlayerControllerFromActorInfo();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();

	// 1인칭 카메라의 화면 중앙에서 상호작용 거리만큼 정면을 탐색합니다.
	const FVector TraceStart = PC->PlayerCameraManager->GetCameraLocation();
	const FVector TraceEnd = TraceStart + PC->PlayerCameraManager->GetCameraRotation().Vector() * InteractionRange;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlayerInteraction), false, AvatarActor);
	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		// 정면에 감지된 Actor가 상호작용 인터페이스를 구현했는지 확인합니다.
		if (AActor* TargetActor = HitResult.GetActor())
		{
			if (IInteractable* Interactable = Cast<IInteractable>(TargetActor))
			{
				// 카메라 전환이 빠르게 끝나도 완료 이벤트를 놓치지 않도록 먼저 대기 Task를 등록합니다.
				UAbilityTask_WaitGameplayEvent* WaitFinishedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
					this,
					TBGameplayTags::GameplayEvent_Interact_Finished,
					nullptr,
					true,
					true);

				WaitFinishedTask->EventReceived.AddDynamic(this, &ThisClass::HandleInteractionFinished);
				WaitFinishedTask->ReadyForActivation();

				// 상호작용이 시작되면 카메라 복귀 완료 시점까지 Ability를 유지합니다.
				if (Interactable->Interact(*PC))
				{
					return;
				}
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
