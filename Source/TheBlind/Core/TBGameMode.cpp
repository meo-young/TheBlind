#include "TBGameMode.h"
#include "TBGameState.h"
#include "Camera/CameraActor.h"
#include "Camera/TBPlayerCameraManager.h"
#include "Character/Player/TBPlayer.h"
#include "Character/Player/TBPlayerController.h"
#include "Character/Player/TBPlayerState.h"
#include "UI/HUD/TBHUD.h"

ATBGameMode::ATBGameMode()
{
	DefaultPawnClass = ATBPlayer::StaticClass();
	PlayerControllerClass = ATBPlayerController::StaticClass();
	PlayerStateClass = ATBPlayerState::StaticClass();
	GameStateClass = ATBGameState::StaticClass();
	HUDClass = ATBHUD::StaticClass(); 
}

bool ATBGameMode::HandlePlayerDeath(const AActor& KillingEnemy, ACameraActor* DeathCamera)
{
	if (bPlayerDead)
	{
		return false;
	}

	bPlayerDead = true;

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (DeathCamera)
	{
		ATBPlayerCameraManager* CameraManager = CastChecked<ATBPlayerCameraManager>(PlayerController->PlayerCameraManager);
		CameraManager->CutToViewTarget(*DeathCamera);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("플레이어 사망 카메라 전환 실패: DeathCamera가 설정되지 않았습니다."));
	}

	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);

	UE_LOG(LogTemp, Warning, TEXT("플레이어 사망: %s에 의해 사망했습니다."), *GetNameSafe(&KillingEnemy));
	return true;
}
