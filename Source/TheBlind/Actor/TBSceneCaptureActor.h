#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TBSceneCaptureActor.generated.h"

class ACameraActor;
class UCameraComponent;
class USceneCaptureComponent2D;

UCLASS()
class THEBLIND_API ATBSceneCaptureActor : public AActor
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────
public:
	ATBSceneCaptureActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;


// ─────────────────────────────────────────────────────────────
// CCTV Capture
// ─────────────────────────────────────────────────────────────
public:
	/** CameraActor의 위치, 회전, FOV를 SceneCapture에 적용하고 화면을 갱신합니다. */
	void CaptureFromCamera(const ACameraActor& CameraActor);

	/** 현재 SceneCapture 위치의 텍스처 스트리밍 요청 활성화 여부를 설정합니다. */
	void SetTextureStreamingViewEnabled(bool bEnabled);

private:
	/** 현재 SceneCapture 위치를 고해상도 텍스처 스트리밍 시점으로 등록합니다. */
	void RegisterTextureStreamingView() const;


// ─────────────────────────────────────────────────────────────
// Components
// ─────────────────────────────────────────────────────────────
protected:
	/** SceneCaptureComponent의 촬영 구도를 에디터에서 확인하기 위한 카메라입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "변수")
	TObjectPtr<UCameraComponent> FakeCameraComponent;

	/** 장면을 RenderTarget으로 촬영하는 SceneCapture 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "변수")
	TObjectPtr<USceneCaptureComponent2D> SceneCaptureComponent;


// ─────────────────────────────────────────────────────────────
// Runtime State
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 SceneCapture 위치를 텍스처 스트리밍 시점으로 등록할지 나타냅니다. */
	bool bTextureStreamingViewEnabled = false;
};
