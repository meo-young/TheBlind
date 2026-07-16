#include "TBRemoteCameraRig.h"
#include "TBCameraComponent.h"

ATBRemoteCameraRig::ATBRemoteCameraRig()
{
	// SceneComponent를 생성합니다.
	{
		SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
		SetRootComponent(SceneRoot);
	}
	
	// 첫 번째 카메라를 생성합니다.
	{
		RemoteIdleCamera = CreateDefaultSubobject<UTBCameraComponent>(TEXT("RemoteIdleCamera"));
		RemoteIdleCamera->SetupAttachment(SceneRoot);	
	}
	
	// 두 번째 카메라를 생성합니다.
	{
		RemoteHoldCamera = CreateDefaultSubobject<UTBCameraComponent>(TEXT("RemoteHoldCamera"));
		RemoteHoldCamera->SetupAttachment(SceneRoot);	
	}
	
	// 세 번째 카메라를 생성합니다.
	{
		RemoteFinalCamera = CreateDefaultSubobject<UTBCameraComponent>(TEXT("RemoteFinalCamera"));
		RemoteFinalCamera->SetupAttachment(SceneRoot);	
	}
}
