#include "TBCameraDirector.h"

ATBCameraDirector::ATBCameraDirector()
{
	PrimaryActorTick.bCanEverTick = true;
	bFindCameraComponentWhenViewTarget = false;
	SetActorEnableCollision(false);
}

void ATBCameraDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 진행 중인 보간이 없다면 현재 시점을 그대로 유지합니다.
	if (!bIsBlending)
	{
		return;
	}

	// 보간 시간에 대한 0~1 범위의 선형 진행도를 계산합니다.
	ElapsedTime += DeltaSeconds;

	const float LinearAlpha = FMath::Clamp(ElapsedTime / BlendDuration, 0.0f, 1.0f);
	const float BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, LinearAlpha, 2.0f);

	// 매 프레임 시작 시점부터 다시 계산하여 누적 보간으로 인한 오차를 방지합니다.
	CurrentView = StartView;

	// BlendViewInfo()가 비 const 참조를 요구하므로 목표 시점의 복사본을 전달합니다.
	FMinimalViewInfo TargetCopy = TargetView;
	CurrentView.BlendViewInfo(TargetCopy, BlendAlpha);

	// 목표 시점에 도착하면 값을 정확히 고정한 뒤 완료 이벤트를 알립니다.
	if (LinearAlpha >= 1.0f)
	{
        CurrentView = TargetView;
        bIsBlending = false;

        OnBlendFinished.Broadcast();
    }
}

void ATBCameraDirector::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	// PlayerCameraManager가 사용할 현재 카메라 시점을 반환합니다.
	OutResult = CurrentView;
}

void ATBCameraDirector::SetViewImmediately(const FMinimalViewInfo& NewView)
{
	// 현재 시점과 다음 보간의 기준을 모두 전달받은 시점으로 맞춥니다.
	CurrentView = NewView;
	StartView = NewView;
	TargetView = NewView;

	// 진행 중이던 보간은 완료 이벤트 없이 즉시 종료합니다.
	ElapsedTime = 0.0f;
	BlendDuration = 0.0f;
	bIsBlending = false;
}

void ATBCameraDirector::BlendTo(const FMinimalViewInfo& NewTargetView, float Duration)
{
	// 유효한 보간 시간이 아니라면 즉시 전환하고 완료 이벤트를 호출합니다.
	if (Duration <= KINDA_SMALL_NUMBER)
	{
		SetViewImmediately(NewTargetView);
		OnBlendFinished.Broadcast();
		return;
    }

	// 보간 도중 방향이 바뀌어도 현재 화면에서 새 목표를 향해 자연스럽게 이어갑니다.
	StartView = CurrentView;
	TargetView = NewTargetView;

	// 새 보간의 시간과 상태를 설정합니다.
	ElapsedTime = 0.0f;
	BlendDuration = Duration;
    bIsBlending = true;
}
float ATBCameraDirector::GetBlendProgress() const
{
	// 보간 중이 아니면 현재 시점에 완전히 도착한 것으로 처리합니다.
	if (!bIsBlending ||
		BlendDuration <= KINDA_SMALL_NUMBER)
    {
        return 1.0f;
    }

	// 보간 도중 역방향 전환 시간을 계산할 때 사용할 수 있는 진행도입니다.
	return FMath::Clamp(ElapsedTime / BlendDuration, 0.0f, 1.0f);
}
