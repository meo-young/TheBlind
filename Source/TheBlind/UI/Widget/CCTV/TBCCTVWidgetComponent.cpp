#include "TBCCTVWidgetComponent.h"

UTBCCTVWidgetComponent::UTBCCTVWidgetComponent()
{
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawSize(FVector2D(1920.0f, 1080.0f));
	SetDrawAtDesiredSize(false);
	SetPivot(FVector2D(0.5f, 0.5f));
	SetWindowFocusable(true);
	bReceiveHardwareInput = true;
	SetVisibility(false);
}

void UTBCCTVWidgetComponent::OnRegister()
{
	bReceiveHardwareInput = true;
	SetCollisionProfileName(TEXT("UI"));
	Super::OnRegister();
}
