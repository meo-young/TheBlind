#include "TBCCTVChannelButton.h"

void UTBCCTVChannelButton::InitializeChannelButton()
{
	OnClicked.RemoveDynamic(this, &ThisClass::HandleClicked);
	OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
}

void UTBCCTVChannelButton::ShutdownChannelButton()
{
	OnClicked.RemoveDynamic(this, &ThisClass::HandleClicked);
}

void UTBCCTVChannelButton::HandleClicked()
{
	if (ChannelIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("CCTV 버튼 클릭 실패: %s에 ChannelIndex가 설정되지 않았습니다."), *GetNameSafe(this));
		return;
	}

	ChannelSelectedEvent.Broadcast(ChannelIndex);
}
