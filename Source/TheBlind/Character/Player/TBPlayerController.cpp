#include "TBPlayerController.h"
#include "TBPlayerState.h"
#include "AbilitySystem/TBAbilitySystemComponent.h"
#include "Camera/TBPlayerCameraManager.h"

ATBPlayerController::ATBPlayerController()
{
	PlayerCameraManagerClass = ATBPlayerCameraManager::StaticClass();
}

void ATBPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (const ATBPlayerState* PS = GetPlayerState<ATBPlayerState>())
	{
		if (UTBAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->ProcessAbilityInput(DeltaTime,bGamePaused);
		}	
	}
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}
