#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TBRemoteCameraRig.generated.h"

class UTBCameraComponent;

UCLASS()
class THEBLIND_API ATBRemoteCameraRig : public AActor
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────	
public:
	ATBRemoteCameraRig();

	
	
// ─────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;
	
public:
	/** 스트리밍 레벨 진입 직후 시점 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTBCameraComponent> RemoteIdleCamera;

	/** 좌클릭 홀드 시 보간되는 시점 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTBCameraComponent> RemoteHoldCamera;

	/** 보간 완료 후 순간 전환되는 시점 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTBCameraComponent> RemoteFinalCamera;

};
