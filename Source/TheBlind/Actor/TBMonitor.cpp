#include "TBMonitor.h"
#include "TBSceneCaptureActor.h"
#include "Camera/CameraActor.h"
#include "Camera/TBPlayerCameraManager.h"
#include "Character/Player/TBPlayerController.h"
#include "UI/Widget/CCTV/TBCCTVWidget.h"

ATBMonitor::ATBMonitor()
{
	Monitor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Monitor"));
	Monitor->SetupAttachment(Mesh);
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

	ATBPlayerCameraManager* CameraManager = Cast<ATBPlayerCameraManager>(PC.PlayerCameraManager);
	if (!IsValid(CameraManager))
	{
		UE_LOG(LogTemp, Error, TEXT("모니터 상호작용 실패: TBPlayerCameraManager를 찾을 수 없습니다."));
		return false;
	}

	ActivePlayerController = &PC;
	CameraManager->OnCameraTransitionFinished().RemoveAll(this);
	CameraManager->OnCameraTransitionFinished().AddUObject(this, &ThisClass::HandleCameraTransitionFinished);

	if (!Super::Interact(PC))
	{
		CameraManager->OnCameraTransitionFinished().RemoveAll(this);
		ActivePlayerController.Reset();
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

void ATBMonitor::HandleCameraTransitionFinished(const ETBCameraTransitionDirection FinishedDirection)
{
	if (FinishedDirection != ETBCameraTransitionDirection::Forward)
	{
		return;
	}

	OpenCCTVWidget();
}

bool ATBMonitor::OpenCCTVWidget()
{
	if (IsValid(CCTVWidget))
	{
		return true;
	}

	ATBPlayerController* PC = ActivePlayerController.Get();
	if (!IsValid(PC))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV Widget 생성 실패: 활성 PlayerController가 유효하지 않습니다."));
		return false;
	}

	if (!CCTVWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV Widget 생성 실패: %s에 CCTVWidgetClass가 설정되지 않았습니다."), *GetNameSafe(this));
		return false;
	}

	CCTVWidget = CreateWidget<UTBCCTVWidget>(PC, CCTVWidgetClass);
	if (!IsValid(CCTVWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV Widget 생성 실패: %s를 생성하지 못했습니다."), *GetNameSafe(CCTVWidgetClass));
		return false;
	}

	CCTVWidget->SetMonitor(this);
	CCTVWidget->AddToViewport();
	return true;
}
