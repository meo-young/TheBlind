#include "TBCCTVWidget.h"
#include "Actor/TBMonitor.h"
#include "Blueprint/WidgetTree.h"
#include "TBCCTVChannelButton.h"

void UTBCCTVWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ChannelButtons.Reset();

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UTBCCTVChannelButton* ChannelButton = Cast<UTBCCTVChannelButton>(Widget);
		if (!IsValid(ChannelButton))
		{
			return;
		}

		ChannelButton->InitializeChannelButton();
		ChannelButton->OnChannelSelected().RemoveAll(this);
		ChannelButton->OnChannelSelected().AddUObject(this, &ThisClass::HandleChannelSelected);
		ChannelButtons.Add(ChannelButton);
	});

	if (ChannelButtons.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV Widget 초기화 실패: CCTV Channel Button이 하나도 없습니다."));
	}
}

void UTBCCTVWidget::NativeDestruct()
{
	for (UTBCCTVChannelButton* ChannelButton : ChannelButtons)
	{
		if (!IsValid(ChannelButton))
		{
			continue;
		}

		ChannelButton->OnChannelSelected().RemoveAll(this);
		ChannelButton->ShutdownChannelButton();
	}

	ChannelButtons.Reset();
	Monitor = nullptr;

	Super::NativeDestruct();
}

void UTBCCTVWidget::SetMonitor(ATBMonitor* InMonitor)
{
	if (!IsValid(InMonitor))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV Widget 초기화 실패: Monitor가 유효하지 않습니다."));
		return;
	}

	Monitor = InMonitor;
}

void UTBCCTVWidget::HandleChannelSelected(const int32 ChannelIndex)
{
	if (!IsValid(Monitor))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV 선택 실패: Widget에 Monitor가 설정되지 않았습니다."));
		return;
	}

	if (ChannelIndex < 0 || ChannelIndex >= Monitor->GetCCTVCount())
	{
		UE_LOG(LogTemp, Warning, TEXT("CCTV 선택 실패: 버튼의 ChannelIndex가 잘못되었습니다. Index=%d, CameraCount=%d"), ChannelIndex, Monitor->GetCCTVCount());
		return;
	}

	Monitor->RequestCCTVSelection(ChannelIndex);
}
