#include "TBCCTVWidget.h"
#include "Actor/TBMonitor.h"
#include "Blueprint/WidgetTree.h"
#include "TBCCTVChannelButton.h"

void UTBCCTVWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ChannelButtons.Reset();

	// WidgetTree의 CCTV 버튼을 수집하고 장소 선택 이벤트를 연결합니다.
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
	// 수집한 CCTV 버튼의 이벤트 연결을 모두 해제합니다.
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

void UTBCCTVWidget::HandleChannelSelected(const ETBLocation Location)
{
	if (!IsValid(Monitor))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV 선택 실패: Widget에 Monitor가 설정되지 않았습니다."));
		return;
	}

	if (!Monitor->HasCCTV(Location))
	{
		UE_LOG(LogTemp, Error, TEXT("CCTV 선택 실패: %s 장소에 유효한 CameraActor가 등록되지 않았습니다."), *UEnum::GetValueAsString(Location));
		return;
	}

	Monitor->RequestCCTVSelection(Location);
}
