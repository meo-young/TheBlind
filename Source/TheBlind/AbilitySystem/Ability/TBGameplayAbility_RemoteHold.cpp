#include "TBGameplayAbility_RemoteHold.h"
#include "Character/Player/TBPlayerController.h"

void UTBGameplayAbility_RemoteHold::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ATBPlayerController* PC = GetTBPlayerControllerFromActorInfo();

	// 실제 카메라 상태 변경은 PlayerController가 담당합니다.
	bRemoteHoldStarted = PC->BeginRemoteHold();
	
	// 아직 RemoteIdle 시점이 아니거나 시퀀스가 유효하지 않다면
	// Ability를 즉시 종료합니다.
	if (!bRemoteHoldStarted)
	{
		K2_EndAbility();
	}
}

void UTBGameplayAbility_RemoteHold::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,bool bReplicateEndAbility, bool bWasCancelled)
{
	ATBPlayerController* PC = GetTBPlayerControllerFromActorInfo();

	// 실제로 시작 요청이 수락된 경우에만 종료를 전달합니다.
	const bool bShouldEndRemoteHold = bRemoteHoldStarted && IsValid(PC);

	bRemoteHoldStarted = false;

	if (bShouldEndRemoteHold)
	{
		PC->EndRemoteHold();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
