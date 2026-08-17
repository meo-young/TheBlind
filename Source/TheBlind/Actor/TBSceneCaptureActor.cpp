#include "TBSceneCaptureActor.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ContentStreaming.h"
#include "Engine/TextureRenderTarget2D.h"

namespace
{
	constexpr float CCTVTextureStreamingBoost = 2.0f;
}

ATBSceneCaptureActor::ATBSceneCaptureActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	FakeCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FakeCameraComponent"));
	SetRootComponent(FakeCameraComponent);

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

void ATBSceneCaptureActor::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RegisterTextureStreamingView();
}

void ATBSceneCaptureActor::SetCaptureEnabled(bool bEnabled)
{
	SceneCaptureComponent->SetActive(bEnabled);
}

bool ATBSceneCaptureActor::CaptureFromCamera(const ACameraActor* CameraActor)
{
	if (!IsValid(CameraActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("CCTV 캡처 실패: CameraActor가 유효하지 않습니다."));
		return false;
	}

	if (!IsValid(SceneCaptureComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV 캡처 실패: %s에 필수 SceneCaptureComponent가 없습니다."), *GetNameSafe(this));
		return false;
	}

	const UCameraComponent* SourceCamera = CameraActor->GetCameraComponent();
	if (!IsValid(SourceCamera))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV 캡처 실패: %s에 필수 CameraComponent가 없습니다."), *GetNameSafe(CameraActor));
		return false;
	}

	SetActorTransform(SourceCamera->GetComponentTransform(), false, nullptr, ETeleportType::TeleportPhysics);
	SceneCaptureComponent->FOVAngle = SourceCamera->FieldOfView;
	SceneCaptureComponent->bCameraCutThisFrame = true;
	SetTextureStreamingViewEnabled(true);
	RegisterTextureStreamingView();

	if (!SceneCaptureComponent->bCaptureEveryFrame)
	{
		SceneCaptureComponent->CaptureScene();
	}

	return true;
}

void ATBSceneCaptureActor::SetTextureStreamingViewEnabled(const bool bEnabled)
{
	bTextureStreamingViewEnabled = bEnabled;
	SetActorTickEnabled(bEnabled);
}

void ATBSceneCaptureActor::RegisterTextureStreamingView() const
{
	if (!bTextureStreamingViewEnabled || !IsValid(SceneCaptureComponent) || !IsValid(SceneCaptureComponent->TextureTarget))
	{
		return;
	}

	const UTextureRenderTarget2D* RenderTarget = SceneCaptureComponent->TextureTarget;
	const float ScreenSize = static_cast<float>(FMath::Max(RenderTarget->SizeX, 1));
	const float HalfFOVRadians = FMath::DegreesToRadians(FMath::Clamp(SceneCaptureComponent->FOVAngle, 5.0f, 170.0f) * 0.5f);
	const float FOVScreenSize = ScreenSize / FMath::Tan(HalfFOVRadians);
	IStreamingManager::Get().AddViewInformation(GetActorLocation(), ScreenSize, FOVScreenSize, CCTVTextureStreamingBoost, false, 0.0f, nullptr, GetWorld());
}
