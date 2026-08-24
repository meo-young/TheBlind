#pragma once

#include "CoreMinimal.h"
#include "Interact/TBPossessableActor.h"
#include "Location/TBLocationTypes.h"
#include "TimerManager.h"
#include "TBMonitor.generated.h"

class ACameraActor;
class ATBSceneCaptureActor;
class UAudioComponent;
class UTBCCTVWidget;
class UTBCCTVWidgetComponent;
class UMaterialInstanceDynamic;
class UTexture2D;
class UWidgetInteractionComponent;

UCLASS()
class THEBLIND_API ATBMonitor : public ATBPossessableActor
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 모니터 구성 요소를 생성합니다. */
	ATBMonitor();

	/** 모니터 머티리얼과 CCTV Widget을 초기화합니다. */
	virtual void BeginPlay() override;

	/** 모니터가 종료될 때 반복 재생 중인 노이즈 사운드를 정리합니다. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


// ─────────────────────────────────────────────────────────────
// Interactable Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 기본 CCTV를 출력하고 플레이어를 모니터 원격 시점으로 전환합니다. */
	virtual bool Interact(ATBPlayerController& PC) override;


// ─────────────────────────────────────────────────────────────
// CCTV
// ─────────────────────────────────────────────────────────────
public:
	/** 노이즈를 표시하는 동안 전달받은 장소의 CCTV로 전환합니다. */
	bool RequestCCTVSelection(ETBLocation Location);

	/** 전달받은 장소에 유효한 CCTV CameraActor가 등록되어 있는지 반환합니다. */
	bool HasCCTV(ETBLocation Location) const;

	/** 현재 출력 중인 CCTV의 장소를 반환합니다. */
	ETBLocation GetCurrentCCTVLocation() const { return CurrentCCTVLocation; }

	/** 현재 출력 중인 CCTV 장소가 설정되었는지 반환합니다. */
	bool HasCurrentCCTVLocation() const { return bHasCurrentCCTVLocation; }

	/** 전달받은 장소를 시청 중이라면 Enemy 이동 노이즈를 표시합니다. */
	void ShowEnemyMovementNoise(ETBLocation Location);

private:
	/** 전달받은 장소에 해당하는 CCTV CameraActor의 시점으로 전환합니다. */
	bool SelectCCTV(ETBLocation Location);

	/** 모니터의 노이즈 화면과 사운드 활성화 여부를 변경합니다. */
	void SetNoiseEnabled(bool bEnabled);

	/** 모니터 위치에서 노이즈 사운드 반복 재생을 시작합니다. */
	void StartNoiseSound();

	/** 현재 재생 중인 모니터 노이즈 사운드를 중단합니다. */
	void StopNoiseSound();

	/** 채널 전환 노이즈를 종료하고 CCTV Widget 입력을 복구합니다. */
	void FinishCCTVChannelTransition();

	/** 진행 중인 채널 전환 타이머를 취소하고 CCTV Widget 입력을 복구합니다. */
	void CancelCCTVChannelTransition();

	/** Enemy 이동 노이즈를 종료하고 CCTV Widget 입력을 복구합니다. */
	void FinishEnemyMovementNoise();

	/** 진행 중인 Enemy 이동 노이즈 타이머를 취소하고 CCTV Widget 입력을 복구합니다. */
	void CancelEnemyMovementNoise();


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
// CCTV Cursor
// ─────────────────────────────────────────────────────────────
public:
	/** 마우스 이동량에 따라 CCTV 화면 내부의 가상 커서를 이동합니다. */
	void MoveCCTVCursor(const FVector2D& CursorDelta);

	/** 가상 커서 위치의 CCTV Widget에 왼쪽 포인터 입력을 시작합니다. */
	void PressCCTVPointer();

	/** CCTV Widget에 전달한 왼쪽 포인터 입력을 종료합니다. */
	void ReleaseCCTVPointer();

private:
	/** 현재 가상 커서 위치를 CCTV Widget 상호작용용 월드 HitResult로 변환합니다. */
	void UpdateCCTVCursorInteraction();

	/** CCTV 가상 커서 입력과 표시를 비활성화합니다. */
	void DisableCCTVCursorInteraction();


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
	UPROPERTY(VisibleAnywhere, Category = "변수|컴포넌트")
	TObjectPtr<UStaticMeshComponent> Monitor;

	/** 모니터 화면 위에 CCTV UI를 렌더링하는 World Space WidgetComponent입니다. */
	UPROPERTY(VisibleAnywhere, Category = "변수|컴포넌트")
	TObjectPtr<UTBCCTVWidgetComponent> CCTVWidgetComponent;

	/** CCTV Widget에 가상 포인터 입력을 전달하는 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, Category = "변수|컴포넌트")
	TObjectPtr<UWidgetInteractionComponent> WidgetInteractionComponent;


// ─────────────────────────────────────────────────────────────
// CCTV Configuration
// ─────────────────────────────────────────────────────────────
protected:
	/** 실제 CCTV 영상을 RenderTarget으로 촬영하는 Actor입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|CCTV")
	TObjectPtr<ATBSceneCaptureActor> CaptureRig;

	/** 장소별 CCTV 시점을 나타내는 CameraActor 맵입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|CCTV")
	TMap<ETBLocation, TObjectPtr<ACameraActor>> CCTVCameras;

	/** 모니터 원격 시점에 진입할 때 처음 출력할 CCTV 장소입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|CCTV")
	ETBLocation InitialCCTVLocation = ETBLocation::Dining;

	/** CCTV 채널 전환과 Enemy 이동 시 노이즈를 표시하는 시간입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|CCTV", meta = (ClampMin = "0.0"))
	float CCTVChannelSwitchNoiseDuration = 0.2f;


// ─────────────────────────────────────────────────────────────
// Cursor Configuration
// ─────────────────────────────────────────────────────────────
protected:
	/** CCTV 화면 안에 표시할 가상 커서 텍스처입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|커서")
	TObjectPtr<UTexture2D> CCTVCursorTexture;

	/** CCTV 가상 커서의 화면 표시 크기입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|커서", meta = (ClampMin = "1.0"))
	FVector2D CCTVCursorSize = FVector2D(64.0f, 64.0f);

	/** 마우스 이동량에 적용할 CCTV 가상 커서 이동 배율입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|커서", meta = (ClampMin = "0.1"))
	float CCTVCursorSensitivity = 2.0f;


// ─────────────────────────────────────────────────────────────
// Runtime State
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 출력 중인 CCTV의 장소입니다. */
	ETBLocation CurrentCCTVLocation = ETBLocation::Dining;

	/** CCTV Widget의 로컬 공간에서 가상 커서가 위치한 좌표입니다. */
	FVector2D CCTVCursorPosition = FVector2D::ZeroVector;

	/** 현재 출력 중인 CCTV 장소가 설정되었는지 나타냅니다. */
	bool bHasCurrentCCTVLocation = false;

	/** 플레이어가 현재 모니터의 원격 화면을 보고 있는지 나타냅니다. */
	bool bRemoteViewActive = false;

	/** CCTV 채널 전환 노이즈 종료에 사용하는 타이머입니다. */
	FTimerHandle CCTVChannelTransitionTimerHandle;

	/** CCTV 채널 전환 노이즈가 진행 중인지 나타냅니다. */
	bool bCCTVChannelTransitionInProgress = false;

	/** Enemy 이동 노이즈 종료에 사용하는 타이머입니다. */
	FTimerHandle EnemyMovementNoiseTimerHandle;

	/** Enemy 이동 노이즈가 진행 중인지 나타냅니다. */
	bool bEnemyMovementNoiseInProgress = false;

	/** 현재 표시 중인 CCTV Widget입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UTBCCTVWidget> CCTVWidget;

	/** 모니터 노이즈 파라미터를 제어하는 Dynamic Material Instance입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> MonitorMaterialInstance;

	/** 현재 반복 재생 중인 모니터 노이즈 AudioComponent입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> NoiseAudioComponent;
};
