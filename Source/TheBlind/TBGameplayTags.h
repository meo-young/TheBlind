#pragma once

#include "NativeGameplayTags.h"

namespace TBGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Interact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_RemoteAction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_RemoteBack);

	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Moving);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Interact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Remote);
	
	
	// Camera State
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CameraState_Player);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CameraState_Entry);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(CameraState_Remote_Final);
	
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEvent_Interact_Finished);
}
