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

void ATBSceneCaptureActor::CaptureFromCamera(const ACameraActor& CameraActor)
{
	const UCameraComponent* SourceCamera = CameraActor.GetCameraComponent();
	SetActorTransform(SourceCamera->GetComponentTransform(), false, nullptr, ETeleportType::TeleportPhysics);
	SceneCaptureComponent->FOVAngle = SourceCamera->FieldOfView;
	SceneCaptureComponent->bCameraCutThisFrame = true;
	SetTextureStreamingViewEnabled(true);
	RegisterTextureStreamingView();

	if (!SceneCaptureComponent->bCaptureEveryFrame)
	{
		SceneCaptureComponent->CaptureScene();
	}
}

void ATBSceneCaptureActor::SetTextureStreamingViewEnabled(const bool bEnabled)
{
	bTextureStreamingViewEnabled = bEnabled;
	SetActorTickEnabled(bEnabled);
}

void ATBSceneCaptureActor::RegisterTextureStreamingView() const
{
	if (!bTextureStreamingViewEnabled || !SceneCaptureComponent->TextureTarget)
	{
		return;
	}

	const UTextureRenderTarget2D* RenderTarget = SceneCaptureComponent->TextureTarget;
	const float ScreenSize = static_cast<float>(FMath::Max(RenderTarget->SizeX, 1));
	const float HalfFOVRadians = FMath::DegreesToRadians(FMath::Clamp(SceneCaptureComponent->FOVAngle, 5.0f, 170.0f) * 0.5f);
	const float FOVScreenSize = ScreenSize / FMath::Tan(HalfFOVRadians);
	IStreamingManager::Get().AddViewInformation(GetActorLocation(), ScreenSize, FOVScreenSize, CCTVTextureStreamingBoost, false, 0.0f, nullptr, GetWorld());
}
