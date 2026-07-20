#include "TBGameplayTags.h"

namespace TBGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Interact, "InputTag.Interact");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_RemoteAction, "InputTag.RemoteAction");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_RemoteBack, "InputTag.RemoteBack");

	
	UE_DEFINE_GAMEPLAY_TAG(Status_Moving, "Status.Moving");
	UE_DEFINE_GAMEPLAY_TAG(Status_Interact, "Status.Interact");
	UE_DEFINE_GAMEPLAY_TAG(Status_Remote, "Status.Remote");
	
	
	UE_DEFINE_GAMEPLAY_TAG(CameraState_Player, "Camera.State.Player");
	UE_DEFINE_GAMEPLAY_TAG(CameraState_Entry, "Camera.State.Entry");
	UE_DEFINE_GAMEPLAY_TAG(CameraState_Remote_Final, "Camera.State.Remote.Final");
	
	
	UE_DEFINE_GAMEPLAY_TAG(GameplayEvent_Interact_Finished, "GameplayEvent.Interact.Finished");
}
