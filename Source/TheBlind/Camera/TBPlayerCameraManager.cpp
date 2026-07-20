#include "TBPlayerCameraManager.h"
#include "Camera/CameraShakeBase.h"

void ATBPlayerCameraManager::StartCameraStepShake(const FTBCameraStep& CameraStep)
{
	StopActiveCameraStepShake(true);

	if (!CameraStep.CameraShakeClass)
	{
		return;
	}

	ActiveCameraStepShake = StartCameraShake(
		CameraStep.CameraShakeClass,
		FMath::Max(0.0f, CameraStep.CameraShakeScale));
}

void ATBPlayerCameraManager::StopActiveCameraStepShake(const bool bImmediately)
{
	if (ActiveCameraStepShake)
	{
		StopCameraShake(ActiveCameraStepShake, bImmediately);
		ActiveCameraStepShake = nullptr;
	}
}

void ATBPlayerCameraManager::BeginCameraTransition(const FTBCameraStep& EntryCam, const FTBCameraStep& FinalCam)
{
	StopActiveCameraStepShake(true);

	ViewTargetStack.Reset(3);
	ViewTargetStack.Push({GetViewTarget(), FViewTargetTransitionParams()});
	ViewTargetStack.Push(EntryCam);
	ViewTargetStack.Push(FinalCam);

	CurrentCameraStep = 0;
	bIsBlendPending = false;
	TransitionDirection = ETBCameraTransitionDirection::Forward;

	OnBlendComplete().RemoveAll(this);
	OnBlendComplete().AddUObject(this, &ThisClass::UpdateCameraTransition);

	UpdateCameraTransition();
}

bool ATBPlayerCameraManager::ReverseCameraTransition()
{
	const bool bCanReverse = TransitionDirection == ETBCameraTransitionDirection::None
		&& !bIsBlendPending
		&& ViewTargetStack.IsValidIndex(CurrentCameraStep)
		&& CurrentCameraStep == ViewTargetStack.Num() - 1;

	if (!bCanReverse)
	{
		return false;
	}
	
	const FTBCameraStep& FinalCameraStep = ViewTargetStack[CurrentCameraStep];
	StopActiveCameraStepShake(FinalCameraStep.bStopCameraShakeImmediately);

	TransitionDirection = ETBCameraTransitionDirection::Reverse;

	OnBlendComplete().RemoveAll(this);
	OnBlendComplete().AddUObject(this, &ThisClass::UpdateCameraTransition);

	UpdateCameraTransition();
	return true;
}

void ATBPlayerCameraManager::UpdateCameraTransition()
{
	bIsBlendPending = false;
	if (TransitionDirection == ETBCameraTransitionDirection::None)
	{
		return;
	}

	const int32 StepDirection = static_cast<int32>(TransitionDirection);
	const int32 NextCameraStep = CurrentCameraStep + StepDirection;
	if (!ViewTargetStack.IsValidIndex(NextCameraStep))
	{
		FinishCameraTransition(true);
		return;
	}

	const FTBCameraStep& CameraStep = ViewTargetStack[NextCameraStep];
	AActor* CurrentViewTarget = CameraStep.ViewTarget.Get();
	if (!IsValid(CurrentViewTarget))
	{
		UE_LOG(LogTemp, Error, TEXT("Camera transition failed: ViewTarget at step %d is invalid."), NextCameraStep);
		FinishCameraTransition(false);
		return;
	}

	// 정방향은 도착 단계, 역방향은 출발 단계의 설정을 사용해야 같은 구간을 반대로 재생할 수 있습니다.
	const int32 TransitionParamsStep = TransitionDirection == ETBCameraTransitionDirection::Forward
		? NextCameraStep
		: CurrentCameraStep;
	const FViewTargetTransitionParams& TransitionParams = ViewTargetStack[TransitionParamsStep].TransitionParams;

	CurrentCameraStep = NextCameraStep;
	UE_LOG(LogTemp, Verbose, TEXT("Starting camera transition step: %d"), CurrentCameraStep);

	SetViewTarget(CurrentViewTarget, TransitionParams);

	if (TransitionParams.BlendTime > KINDA_SMALL_NUMBER)
	{
		bIsBlendPending = true;
		return;
	}

	// 보간 시간이 0인 경우 즉시 업데이트 합니다.
	SetGameCameraCutThisFrame();
	UpdateCameraTransition();
}

void ATBPlayerCameraManager::FinishCameraTransition(const bool bCompleted)
{
	const ETBCameraTransitionDirection FinishedDirection = TransitionDirection;

	bIsBlendPending = false;
	TransitionDirection = ETBCameraTransitionDirection::None;
	OnBlendComplete().RemoveAll(this);

	if (!bCompleted)
	{
		ViewTargetStack.Reset();
		CurrentCameraStep = 0;
		return;
	}

	if (FinishedDirection == ETBCameraTransitionDirection::Forward)
	{
		if (ViewTargetStack.IsValidIndex(CurrentCameraStep))
		{
			StartCameraStepShake(ViewTargetStack[CurrentCameraStep]);
		}
	}
	else if (FinishedDirection == ETBCameraTransitionDirection::Reverse)
	{
		ViewTargetStack.Reset();
		CurrentCameraStep = 0;
	}

	CameraTransitionFinishedEvent.Broadcast(FinishedDirection);
}
