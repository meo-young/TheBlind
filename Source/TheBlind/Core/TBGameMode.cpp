#include "TBGameMode.h"
#include "TBGameState.h"
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
