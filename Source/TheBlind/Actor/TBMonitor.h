#pragma once

#include "CoreMinimal.h"
#include "Interact/TBPossessableActor.h"
#include "TimerManager.h"
#include "TBMonitor.generated.h"

class ACameraActor;
class ATBSceneCaptureActor;
class UTBCCTVWidget;
class UTBCCTVWidgetComponent;
class UMaterialInstanceDynamic;

UCLASS()
class THEBLIND_API ATBMonitor : public ATBPossessableActor
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────
public:
	ATBMonitor();
	virtual void BeginPlay() override;


// ─────────────────────────────────────────────────────────────
// Interactable Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual bool Interact(ATBPlayerController& PC) override;


// ─────────────────────────────────────────────────────────────
// CCTV
// ─────────────────────────────────────────────────────────────
public:
	/** 노이즈를 표시하는 동안 전달받은 CCTV 채널로 전환합니다. */
	bool RequestCCTVSelection(int32 Index);

	/** 선택 가능한 CCTV 채널 수를 반환합니다. */
	int32 GetCCTVCount() const { return CCTVCameras.Num(); }

private:
	/** 전달받은 인덱스에 해당하는 CCTV CameraActor의 시점으로 전환합니다. */
	bool SelectCCTV(int32 Index);

	/** 모니터 머티리얼의 NoiseEnabled 파라미터를 변경합니다. */
	void SetNoiseEnabled(bool bEnabled);

	/** 채널 전환 노이즈를 종료하고 CCTV Widget 입력을 복구합니다. */
	void FinishCCTVChannelTransition();

	/** 진행 중인 채널 전환 타이머를 취소하고 CCTV Widget 입력을 복구합니다. */
	void CancelCCTVChannelTransition();


// ─────────────────────────────────────────────────────────────
// Remote View
// ─────────────────────────────────────────────────────────────
public:
	/** 모니터 원격 시점 진입이 완료된 후 CCTV 화면과 UI를 활성화합니다. */
	void HandleRemoteViewEntered();

	/** 모니터 원격 시점 종료가 시작될 때 노이즈를 활성화합니다. */
	void HandleRemoteViewExitStarted();

	/** 모니터 원격 시점 종료가 완료된 후 CCTV 화면과 UI를 비활성화합니다. */
	void HandleRemoteViewExited();


// ─────────────────────────────────────────────────────────────
// CCTV Widget
// ─────────────────────────────────────────────────────────────
private:
	/** 모니터의 CCTV Widget을 표시하고 입력을 활성화합니다. */
	void OpenCCTVWidget();

	/** 모니터의 CCTV Widget을 숨기고 입력을 비활성화합니다. */
	void CloseCCTVWidget();


// ─────────────────────────────────────────────────────────────
// Components
// ─────────────────────────────────────────────────────────────
protected:
	/** 집 안의 여러 장소를 렌더링하는 모니터 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Monitor;

	/** 모니터 화면 위에 CCTV UI를 렌더링하는 World Space WidgetComponent입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTBCCTVWidgetComponent> CCTVWidgetComponent;


// ─────────────────────────────────────────────────────────────
// CCTV Configuration
// ─────────────────────────────────────────────────────────────
protected:
	/** 실제 CCTV 영상을 RenderTarget으로 촬영하는 Actor입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|CCTV")
	TObjectPtr<ATBSceneCaptureActor> CaptureRig;

	/** 각 장소의 CCTV 시점을 나타내는 CameraActor 배열입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|CCTV")
	TArray<TObjectPtr<ACameraActor>> CCTVCameras;

	/** CCTV 채널 전환 시 노이즈를 표시하는 시간입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|CCTV", meta = (ClampMin = "0.0"))
	float CCTVChannelSwitchNoiseDuration = 0.2f;


// ─────────────────────────────────────────────────────────────
// Runtime State
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 출력 중인 CCTV CameraActor의 인덱스입니다. */
	int32 CurrentCCTVIndex = INDEX_NONE;

	/** CCTV 채널 전환 노이즈 종료에 사용하는 타이머입니다. */
	FTimerHandle CCTVChannelTransitionTimerHandle;

	/** CCTV 채널 전환 노이즈가 진행 중인지 나타냅니다. */
	bool bCCTVChannelTransitionInProgress = false;

	/** 현재 표시 중인 CCTV Widget입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UTBCCTVWidget> CCTVWidget;

	/** 모니터 노이즈 파라미터를 제어하는 Dynamic Material Instance입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MonitorMaterialInstance;
};
