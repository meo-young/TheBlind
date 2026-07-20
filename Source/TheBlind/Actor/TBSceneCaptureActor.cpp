#include "TBSceneCaptureActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"

ATBSceneCaptureActor::ATBSceneCaptureActor()
{
	FakeCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FakeCameraComponent"));
	FakeCameraComponent->SetupAttachment(GetRootComponent());

	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent->SetupAttachment(FakeCameraComponent);

	// SceneColor는 포스트 프로세스 이전 버퍼이므로 FinalColor를 캡처해야 후처리 결과가 포함됩니다.
	SceneCaptureComponent->CaptureSource = SCS_FinalColorLDR;
	SceneCaptureComponent->ShowFlags.SetPostProcessing(true);
	SceneCaptureComponent->PostProcessBlendWeight = 1.0f;

	SceneCaptureComponent->SetActive(true);
}

void ATBSceneCaptureActor::BeginPlay()
{
	Super::BeginPlay();
	
	SceneCaptureComponent->SetActive(true);
	SceneCaptureComponent->CaptureScene();
}

void ATBSceneCaptureActor::SetCaptureEnabled(bool bEnabled)
{
	SceneCaptureComponent->SetActive(bEnabled);
}
