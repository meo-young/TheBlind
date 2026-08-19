#include "TBEnemyGimmickDevice.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/TBPlayerController.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "TBGameplayTags.h"

bool ATBEnemyGimmickDevice::Interact(ATBPlayerController& PC)
{
	Super::Interact(PC);

	// Enemy가 발견되면 성공 연출을 시작합니다.
	if (EnemyDirector->TryBeginEnemyRelocation(TargetLocation))
	{
		SuccessSequenceActor->GetSequencePlayer()->Play();
	}

	// 대상 장소에 Enemy가 없으면 기획될 페널티와 실패 연출을 실행합니다.
	else
	{
		FailureSequenceActor->GetSequencePlayer()->Play();
	}

	// 카메라 전환이 없는 즉시 상호작용이므로 완료 이벤트를 바로 전달합니다.
	APawn* PlayerPawn = PC.GetPawn();
	FGameplayEventData Payload;
	Payload.EventTag = TBGameplayTags::GameplayEvent_Interact_Finished;
	Payload.Instigator = PlayerPawn;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(PlayerPawn, TBGameplayTags::GameplayEvent_Interact_Finished, Payload);

	return true;
}
