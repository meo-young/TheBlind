#include "TBCCTVWidgetComponent.h"

UTBCCTVWidgetComponent::UTBCCTVWidgetComponent()
{
	// 모니터 패널의 월드 크기와 별개인 위젯 렌더 해상도를 설정합니다.
	{
		SetWidgetSpace(EWidgetSpace::World);
		SetDrawSize(FVector2D(1920.0f, 1080.0f));
		SetDrawAtDesiredSize(false);
		SetPivot(FVector2D(0.5f, 0.5f));
	}

	// 모니터 전용 가상 포인터만 사용하도록 실제 마우스 입력을 차단합니다.
	{
		SetWindowFocusable(false);
		bReceiveHardwareInput = false;
	}

	SetVisibility(false);
}

void UTBCCTVWidgetComponent::OnRegister()
{
	// 월드 공간 Widget의 충돌만 가상 포인터 HitResult에 사용합니다.
	bReceiveHardwareInput = false;
	SetCollisionProfileName(TEXT("UI"));
	Super::OnRegister();
}
