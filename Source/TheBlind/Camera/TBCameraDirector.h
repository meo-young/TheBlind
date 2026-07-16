#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraTypes.h"
#include "GameFramework/Actor.h"
#include "TBCameraDirector.generated.h"

UCLASS(Transient, NotPlaceable)
class THEBLIND_API ATBCameraDirector : public AActor
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────
public:
	ATBCameraDirector();
	virtual void Tick(float DeltaSeconds) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;



// ─────────────────────────────────────────────────────────────
// Camera View
// ─────────────────────────────────────────────────────────────
public:
	/** 현재 출력할 시점을 즉시 설정하고 진행 중인 보간을 종료합니다. */
	void SetViewImmediately(const FMinimalViewInfo& NewView);

	/** 현재 시점에서 목표 시점까지 지정된 시간 동안 보간합니다. */
	void BlendTo(const FMinimalViewInfo& NewTargetView, float Duration);

	/** 현재 진행 중인 보간의 진행도를 0~1 범위로 반환합니다. */
	float GetBlendProgress() const;



// ─────────────────────────────────────────────────────────────
// Delegate
// ─────────────────────────────────────────────────────────────
public:
	/** 보간이 목표 시점에 도착했을 때 호출됩니다. */
	FSimpleMulticastDelegate OnBlendFinished;



// ─────────────────────────────────────────────────────────────
// State
// ─────────────────────────────────────────────────────────────
public:
	/** 현재 카메라 시점이 보간 중인지 나타냅니다. */
	bool bIsBlending = false;



// ─────────────────────────────────────────────────────────────
// View Data
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 프레임에 실제로 출력하는 카메라 시점입니다. */
	FMinimalViewInfo CurrentView;

	/** 보간을 시작한 순간의 카메라 시점입니다. */
	FMinimalViewInfo StartView;

	/** 보간이 완료되었을 때 도달해야 하는 카메라 시점입니다. */
	FMinimalViewInfo TargetView;



// ─────────────────────────────────────────────────────────────
// Blend Data
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 보간이 시작된 후 경과한 시간입니다. */
	float ElapsedTime = 0.0f;

	/** 현재 보간을 완료하는 데 필요한 전체 시간입니다. */
	float BlendDuration = 0.0f;

};
