#include "TBMonitor.h"
#include "TBSceneCaptureActor.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
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
	Monitor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Monitor"));
	Monitor->SetupAttachment(Mesh);

	CCTVWidgetComponent = CreateDefaultSubobject<UTBCCTVWidgetComponent>(TEXT("CCTVWidgetComponent"));
	CCTVWidgetComponent->SetupAttachment(Monitor);
}

void ATBMonitor::BeginPlay()
{
	Super::BeginPlay();

	MonitorMaterialInstance = Monitor->CreateDynamicMaterialInstance(0);
	if (!IsValid(MonitorMaterialInstance))
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

	CCTVWidgetComponent->InitWidget();
	CCTVWidget = Cast<UTBCCTVWidget>(CCTVWidgetComponent->GetUserWidgetObject());
	if (!IsValid(CCTVWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV Widget 초기화 실패: %s의 CCTVWidgetComponent에 TBCCTVWidget 기반 Widget Class가 설정되지 않았습니다."), *GetNameSafe(this));
		return;
	}

	CCTVWidget->SetMonitor(this);
	CCTVWidgetComponent->SetVisibility(false);
}

bool ATBMonitor::Interact(ATBPlayerController& PC)
{
	if (CCTVCameras.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("모니터 상호작용 실패: %s에 CCTV CameraActor가 설정되지 않았습니다."), *GetNameSafe(this));
		return false;
	}

	if (!SelectCCTV(0))
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

bool ATBMonitor::SelectCCTV(const int32 Index)
{
	if (!IsValid(CaptureRig))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV 선택 실패: %s에 필수 CaptureRig가 지정되지 않았습니다."), *GetNameSafe(this));
		return false;
	}

	if (!CCTVCameras.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("CCTV 선택 실패: 잘못된 인덱스입니다. Index=%d, CameraCount=%d"), Index, CCTVCameras.Num());
		return false;
	}

	ACameraActor* SelectedCamera = CCTVCameras[Index];
	if (!IsValid(SelectedCamera))
	{
		UE_LOG(LogTemp, Warning, TEXT("CCTV 선택 실패: CCTVCameras[%d]가 지정되지 않았습니다."), Index);
		return false;
	}

	if (!CaptureRig->CaptureFromCamera(SelectedCamera))
	{
		return false;
	}

	CurrentCCTVIndex = Index;
	return true;
}

bool ATBMonitor::RequestCCTVSelection(const int32 Index)
{
	if (Index == CurrentCCTVIndex)
	{
		return true;
	}

	if (bCCTVChannelTransitionInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("CCTV 선택 무시: 채널 전환이 이미 진행 중입니다. Index=%d"), Index);
		return false;
	}

	if (!SetNoiseEnabled(true))
	{
		return false;
	}

	bCCTVChannelTransitionInProgress = true;
	if (IsValid(CCTVWidget))
	{
		CCTVWidget->SetIsEnabled(false);
	}
	CloseCCTVWidget();

	if (!SelectCCTV(Index))
	{
		FinishCCTVChannelTransition();
		return false;
	}

	if (CCTVChannelSwitchNoiseDuration <= 0.0f)
	{
		FinishCCTVChannelTransition();
		return true;
	}

	GetWorldTimerManager().SetTimer(CCTVChannelTransitionTimerHandle, this, &ThisClass::FinishCCTVChannelTransition, CCTVChannelSwitchNoiseDuration, false);
	return true;
}

bool ATBMonitor::SetNoiseEnabled(const bool bEnabled)
{
	if (!IsValid(MonitorMaterialInstance))
	{
		UE_LOG(LogTemp, Error, TEXT("모니터 노이즈 변경 실패: %s의 Dynamic Material Instance가 유효하지 않습니다."), *GetNameSafe(this));
		return false;
	}

	MonitorMaterialInstance->SetScalarParameterValue(NoiseEnabledParameterName, bEnabled ? 1.0f : 0.0f);
	bNoiseEnabled = bEnabled;
	return true;
}

void ATBMonitor::HandleRemoteViewEntered()
{
	if (IsValid(CaptureRig))
	{
		CaptureRig->SetTextureStreamingViewEnabled(true);
	}

	SetNoiseEnabled(false);
	OpenCCTVWidget();
}

void ATBMonitor::HandleRemoteViewExitStarted()
{
	CancelCCTVChannelTransition();
	if (IsValid(CaptureRig))
	{
		CaptureRig->SetTextureStreamingViewEnabled(false);
	}
	SetNoiseEnabled(true);
	CloseCCTVWidget();
}

void ATBMonitor::HandleRemoteViewExited()
{
	CancelCCTVChannelTransition();
	if (IsValid(CaptureRig))
	{
		CaptureRig->SetTextureStreamingViewEnabled(false);
	}
	CloseCCTVWidget();
}

void ATBMonitor::FinishCCTVChannelTransition()
{
	GetWorldTimerManager().ClearTimer(CCTVChannelTransitionTimerHandle);
	bCCTVChannelTransitionInProgress = false;
	if (IsValid(CCTVWidget))
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
	if (IsValid(CCTVWidget))
	{
		CCTVWidget->SetIsEnabled(true);
	}
}

bool ATBMonitor::OpenCCTVWidget()
{
	if (!IsValid(CCTVWidgetComponent) || !IsValid(CCTVWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV Widget 표시 실패: %s의 CCTVWidgetComponent가 초기화되지 않았습니다."), *GetNameSafe(this));
		return false;
	}

	CCTVWidgetComponent->SetHiddenInGame(false);
	CCTVWidgetComponent->SetVisibility(true, true);
	CCTVWidgetComponent->SetComponentTickEnabled(true);
	CCTVWidgetComponent->RequestRedraw();
	return true;
}

void ATBMonitor::CloseCCTVWidget()
{
	if (!IsValid(CCTVWidgetComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV Widget 숨김 실패: %s의 CCTVWidgetComponent가 유효하지 않습니다."), *GetNameSafe(this));
		return;
	}

	CCTVWidgetComponent->SetVisibility(false, true);
}
