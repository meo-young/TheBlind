#include "TBCCTVChannelButton.h"

void UTBCCTVChannelButton::InitializeChannelButton()
{
	// 중복 바인딩을 제거한 뒤 버튼 클릭 콜백을 연결합니다.
	OnClicked.RemoveDynamic(this, &ThisClass::HandleClicked);
	OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
}

void UTBCCTVChannelButton::ShutdownChannelButton()
{
	OnClicked.RemoveDynamic(this, &ThisClass::HandleClicked);
}

void UTBCCTVChannelButton::HandleClicked()
{
	ChannelSelectedEvent.Broadcast(TargetLocation);
}
