#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "TBCameraComponent.generated.h"

class UCameraShakeBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class THEBLIND_API UTBCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Blend
// ─────────────────────────────────────────────────────────────
public:
	UPROPERTY(EditAnywhere, Category = "변수")
	FViewTargetTransitionParams ViewTargetTransitionParams;



// ─────────────────────────────────────────────────────────────
// Camera Shake
// ─────────────────────────────────────────────────────────────
public:
	/** 이 카메라로 전환을 마쳤을 때 재생할 UCameraShakeBase 클래스입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|카메라 흔들림")
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	/** CameraShakeClass에 적용할 전체 세기 배율입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|카메라 흔들림", meta = (ClampMin = "0.0", EditCondition = "CameraShakeClass != nullptr"))
	float CameraShakeScale = 1.0f;

	/** 이 카메라를 떠날 때 클래스의 Blend Out을 무시하고 즉시 중지할지 여부입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|카메라 흔들림", meta = (EditCondition = "CameraShakeClass != nullptr"))
	bool bStopCameraShakeImmediately = false;
	
};
