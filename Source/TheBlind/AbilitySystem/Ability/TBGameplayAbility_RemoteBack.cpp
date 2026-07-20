#include "TBGameplayAbility_RemoteBack.h"
#include "Camera/TBPlayerCameraManager.h"
#include "Character/Player/TBPlayerController.h"

void UTBGameplayAbility_RemoteBack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (ATBPlayerController* PC = GetTBPlayerControllerFromActorInfo())
	{
		if (ATBPlayerCameraManager* PCM = Cast<ATBPlayerCameraManager>(PC->PlayerCameraManager))
		{
			PCM->ReverseCameraTransition();
		}
	}
	
	K2_EndAbility();
}
