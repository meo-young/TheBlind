#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "TBPlayerCameraManager.generated.h"

class UCameraShakeBase;

USTRUCT()
struct FTBCameraStep
{
	GENERATED_BODY()

	/** 현재 카메라의 ViewTarget */
	TWeakObjectPtr<AActor> ViewTarget;

	/** 이전 단계와 이 단계 사이의 전환 설정 */
	FViewTargetTransitionParams TransitionParams;

	/** 이 단계에 완전히 도착했을 때 재생할 카메라 흔들림 클래스입니다. */
	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	/** 카메라 흔들림 클래스에 적용할 배율입니다. */
	float CameraShakeScale = 1.0f;

	/** 이 단계를 떠날 때 Blend Out 없이 즉시 흔들림을 중지할지 여부입니다. */
	bool bStopCameraShakeImmediately = false;
};

enum class ETBCameraTransitionDirection : int8
{
	None = 0,
	Forward = 1,
	Reverse = -1
};

DECLARE_MULTICAST_DELEGATE_OneParam(FTBCameraTransitionFinished, ETBCameraTransitionDirection);

UCLASS()
class THEBLIND_API ATBPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Transition State
// ─────────────────────────────────────────────────────────────
public:
	/** 카메라 보간을 시작하는 함수입니다. */
	void BeginCameraTransition(const FTBCameraStep& EntryCam, const FTBCameraStep& FinalCam);
	
	/** 역순으로 카메라 보간을 시작하는 함수입니다. */
	bool ReverseCameraTransition();

	FTBCameraTransitionFinished& OnCameraTransitionFinished()
	{
		return CameraTransitionFinishedEvent;
	}

private:
	/** 해당 카메라 단계에 설정된 UCameraShakeBase를 재생합니다. */
	void StartCameraStepShake(const FTBCameraStep& CameraStep);

	/** 현재 카메라 단계가 재생한 UCameraShakeBase 인스턴스를 중지합니다. */
	void StopActiveCameraStepShake(bool bImmediately);

	/** 현재 인덱스에 맞는 ViewTarget을 업데이트 하는 함수입니다. */
	void UpdateCameraTransition();
	
	/** 보간이 모두 끝났을 때 호출되는 함수입니다. */
	void FinishCameraTransition(bool bCompleted);

	
	
// ─────────────────────────────────────────────────────────────
// Transition State
// ─────────────────────────────────────────────────────────────
private:
	/** 보간을 진행해야 하는 ViewTarget 모음 배열입니다. */
	UPROPERTY()
	TArray<FTBCameraStep> ViewTargetStack;

	/** 현재 보간 중인 ViewTarget 인덱스입니다. */
	int32 CurrentCameraStep = 0;
	
	/** 보간 중임을 나타내는 상태 변수입니다. */
	bool bIsBlendPending = false;

	ETBCameraTransitionDirection TransitionDirection = ETBCameraTransitionDirection::None;
	FTBCameraTransitionFinished CameraTransitionFinishedEvent;

	/** 현재 카메라 전환 단계가 재생한 흔들림 인스턴스입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UCameraShakeBase> ActiveCameraStepShake;
	
};
