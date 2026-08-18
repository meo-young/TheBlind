#include "TBMonitor.h"
#include "TBSceneCaptureActor.h"
#include "Camera/CameraActor.h"
#include "Character/Player/TBPlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/Widget/CCTV/TBCCTVWidget.h"
#include "UI/Widget/CCTV/TBCCTVWidgetComponent.h"

namespace
{
	const FName NoiseEnabledParameterName(TEXT("NoiseEnabled"));
}

ATBMonitor::ATBMonitor()
{
	// 모니터 메시를 생성합니다.
	{
		Monitor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Monitor"));
		Monitor->SetupAttachment(Mesh);
	}

	// CCTV Widget 컴포넌트를 생성합니다.
	{
		CCTVWidgetComponent = CreateDefaultSubobject<UTBCCTVWidgetComponent>(TEXT("CCTVWidgetComponent"));
		CCTVWidgetComponent->SetupAttachment(Monitor);
	}
}

void ATBMonitor::BeginPlay()
{
	Super::BeginPlay();

	// 모니터 머티리얼의 노이즈 파라미터를 제어할 인스턴스를 준비합니다.
	MonitorMaterialInstance = Monitor->CreateDynamicMaterialInstance(0);
	if (!MonitorMaterialInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("모니터 노이즈 초기화 실패: %s의 Element 0에서 Dynamic Material Instance를 생성하지 못했습니다."), *GetNameSafe(this));
	}
	else
	{
		float NoiseEnabledValue = 0.0f;
		if (!MonitorMaterialInstance->GetScalarParameterValue(FMaterialParameterInfo(NoiseEnabledParameterName), NoiseEnabledValue))
		{
			UE_LOG(LogTemp, Error, TEXT("모니터 노이즈 초기화 실패: %s의 머티리얼에 Scalar Parameter 'NoiseEnabled'가 없습니다."), *GetNameSafe(this));
			MonitorMaterialInstance = nullptr;
		}
		else
		{
			SetNoiseEnabled(true);
		}
	}

	// CCTV Widget을 생성하고 현재 Monitor를 선택 요청 대상으로 연결합니다.
	CCTVWidgetComponent->InitWidget();
	CCTVWidget = Cast<UTBCCTVWidget>(CCTVWidgetComponent->GetUserWidgetObject());
	if (!CCTVWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV Widget 초기화 실패: %s의 CCTVWidgetComponent에 TBCCTVWidget 기반 Widget Class가 설정되지 않았습니다."), *GetNameSafe(this));
		return;
	}

	CCTVWidget->SetMonitor(this);
	CCTVWidgetComponent->SetVisibility(false);
}

bool ATBMonitor::Interact(ATBPlayerController& PC)
{
	// 설정된 기본 장소의 CCTV를 먼저 출력한 뒤 원격 시점 전환을 시작합니다.
	if (CCTVCameras.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("모니터 상호작용 실패: %s에 CCTV CameraActor가 설정되지 않았습니다."), *GetNameSafe(this));
		return false;
	}

	if (!SelectCCTV(InitialCCTVLocation))
	{
		return false;
	}

	PC.SetActiveMonitor(this);
	if (!Super::Interact(PC))
	{
		PC.SetActiveMonitor(nullptr);
		CaptureRig->SetTextureStreamingViewEnabled(false);
		return false;
	}

	return true;
}

bool ATBMonitor::RequestCCTVSelection(const ETBLocation Location)
{
	// 같은 장소의 재선택과 전환 중 중복 요청을 무시합니다.
	if (bHasCurrentCCTVLocation && Location == CurrentCCTVLocation)
	{
		return true;
	}

	if (bCCTVChannelTransitionInProgress || bEnemyMovementNoiseInProgress)
	{
		return false;
	}

	// 노이즈가 표시되는 동안 카메라를 변경하고 Widget 입력을 차단합니다.
	SetNoiseEnabled(true);
	bCCTVChannelTransitionInProgress = true;
	if (CCTVWidget)
	{
		CCTVWidget->SetIsEnabled(false);
	}
	CloseCCTVWidget();

	if (!SelectCCTV(Location))
	{
		FinishCCTVChannelTransition();
		return false;
	}

	if (CCTVChannelSwitchNoiseDuration <= 0.0f)
	{
		FinishCCTVChannelTransition();
		return true;
	}

	// 설정한 노이즈 시간이 지난 뒤 CCTV 입력과 화면을 복구합니다.
	GetWorldTimerManager().SetTimer(CCTVChannelTransitionTimerHandle, this, &ThisClass::FinishCCTVChannelTransition, CCTVChannelSwitchNoiseDuration, false);
	return true;
}

bool ATBMonitor::HasCCTV(const ETBLocation Location) const
{
	const TObjectPtr<ACameraActor>* Camera = CCTVCameras.Find(Location);
	return Camera && IsValid(Camera->Get());
}

void ATBMonitor::ShowEnemyMovementNoise(const ETBLocation Location)
{
	if (!bRemoteViewActive || !bHasCurrentCCTVLocation || CurrentCCTVLocation != Location || bCCTVChannelTransitionInProgress)
	{
		return;
	}

	// 같은 장소에서 Enemy가 연속으로 이동하면 기존 타이머를 갱신하여 노이즈 시간을 연장합니다.
	SetNoiseEnabled(true);
	bEnemyMovementNoiseInProgress = true;
	if (CCTVWidget)
	{
		CCTVWidget->SetIsEnabled(false);
	}
	CloseCCTVWidget();

	if (CCTVChannelSwitchNoiseDuration <= 0.0f)
	{
		FinishEnemyMovementNoise();
		return;
	}

	GetWorldTimerManager().SetTimer(EnemyMovementNoiseTimerHandle, this, &ThisClass::FinishEnemyMovementNoise, CCTVChannelSwitchNoiseDuration, false);
}

bool ATBMonitor::SelectCCTV(const ETBLocation Location)
{
	if (!CaptureRig)
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV 선택 실패: %s에 필수 CaptureRig가 지정되지 않았습니다."), *GetNameSafe(this));
		return false;
	}

	// 장소에 연결된 카메라를 찾아 CaptureRig에 적용합니다.
	const TObjectPtr<ACameraActor>* Camera = CCTVCameras.Find(Location);
	ACameraActor* SelectedCamera = nullptr;
	if (Camera)
	{
		SelectedCamera = Camera->Get();
	}

	if (!IsValid(SelectedCamera))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV 선택 실패: %s 장소에 CameraActor가 지정되지 않았습니다."), *UEnum::GetValueAsString(Location));
		return false;
	}

	CaptureRig->CaptureFromCamera(*SelectedCamera);
	CurrentCCTVLocation = Location;
	bHasCurrentCCTVLocation = true;
	return true;
}

void ATBMonitor::SetNoiseEnabled(const bool bEnabled)
{
	if (MonitorMaterialInstance)
	{
		float NoiseEnabledValue = 0.0f;
		if (bEnabled)
		{
			NoiseEnabledValue = 1.0f;
		}

		MonitorMaterialInstance->SetScalarParameterValue(NoiseEnabledParameterName, NoiseEnabledValue);
	}
}

void ATBMonitor::FinishCCTVChannelTransition()
{
	GetWorldTimerManager().ClearTimer(CCTVChannelTransitionTimerHandle);
	bCCTVChannelTransitionInProgress = false;
	if (CCTVWidget)
	{
		CCTVWidget->SetIsEnabled(true);
	}
	SetNoiseEnabled(false);
	OpenCCTVWidget();
}

void ATBMonitor::CancelCCTVChannelTransition()
{
	GetWorldTimerManager().ClearTimer(CCTVChannelTransitionTimerHandle);
	bCCTVChannelTransitionInProgress = false;
	if (CCTVWidget)
	{
		CCTVWidget->SetIsEnabled(true);
	}
}

void ATBMonitor::FinishEnemyMovementNoise()
{
	GetWorldTimerManager().ClearTimer(EnemyMovementNoiseTimerHandle);
	bEnemyMovementNoiseInProgress = false;
	if (!bRemoteViewActive)
	{
		return;
	}

	if (CCTVWidget)
	{
		CCTVWidget->SetIsEnabled(true);
	}
	SetNoiseEnabled(false);
	OpenCCTVWidget();
}

void ATBMonitor::CancelEnemyMovementNoise()
{
	GetWorldTimerManager().ClearTimer(EnemyMovementNoiseTimerHandle);
	bEnemyMovementNoiseInProgress = false;
	if (CCTVWidget)
	{
		CCTVWidget->SetIsEnabled(true);
	}
}

void ATBMonitor::HandleRemoteViewEntered()
{
	bRemoteViewActive = true;
	CaptureRig->SetTextureStreamingViewEnabled(true);
	SetNoiseEnabled(false);
	OpenCCTVWidget();
}

void ATBMonitor::HandleRemoteViewExitStarted()
{
	bRemoteViewActive = false;
	CancelCCTVChannelTransition();
	CancelEnemyMovementNoise();
	CaptureRig->SetTextureStreamingViewEnabled(false);
	SetNoiseEnabled(true);
	CloseCCTVWidget();
}

void ATBMonitor::HandleRemoteViewExited()
{
	bRemoteViewActive = false;
	CancelCCTVChannelTransition();
	CancelEnemyMovementNoise();
	CaptureRig->SetTextureStreamingViewEnabled(false);
	CloseCCTVWidget();
}

void ATBMonitor::OpenCCTVWidget()
{
	if (!CCTVWidget)
	{
		return;
	}

	CCTVWidgetComponent->SetHiddenInGame(false);
	CCTVWidgetComponent->SetVisibility(true, true);
	CCTVWidgetComponent->SetComponentTickEnabled(true);
	CCTVWidgetComponent->RequestRedraw();
}

void ATBMonitor::CloseCCTVWidget()
{
	CCTVWidgetComponent->SetVisibility(false, true);
}
