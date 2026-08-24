#include "TBCCTVWidget.h"
#include "Actor/TBMonitor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
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
	CCTVCursorImage = nullptr;
	CCTVCursorSlot = nullptr;
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

FVector UTBCCTVWidget::GetMonitorLocation() const
{
	return Monitor->GetActorLocation();
}

void UTBCCTVWidget::InitializeCCTVCursor(UTexture2D& CursorTexture, const FVector2D& CursorSize)
{
	// CCTV Widget의 루트 Canvas에 입력을 가로채지 않는 커서 이미지를 생성합니다.
	UCanvasPanel* RootCanvas = CastChecked<UCanvasPanel>(GetRootWidget());
	RootCanvas->SetClipping(EWidgetClipping::ClipToBounds);
	CCTVCursorImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("CCTVCursor"));
	CCTVCursorImage->SetBrushFromTexture(&CursorTexture, true);
	CCTVCursorImage->SetVisibility(ESlateVisibility::Collapsed);

	CCTVCursorSlot = RootCanvas->AddChildToCanvas(CCTVCursorImage);
	CCTVCursorSlot->SetSize(CursorSize);
	CCTVCursorSlot->SetZOrder(100);
}

void UTBCCTVWidget::SetCCTVCursorPosition(const FVector2D& CursorPosition)
{
	CCTVCursorSlot->SetPosition(CursorPosition);
}

void UTBCCTVWidget::SetCCTVCursorVisible(const bool bVisible)
{
	// 표시 중에도 아래의 CCTV 버튼이 포인터 입력을 받을 수 있도록 Hit Test를 비활성화합니다.
	if (bVisible)
	{
		CCTVCursorImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		return;
	}

	CCTVCursorImage->SetVisibility(ESlateVisibility::Collapsed);
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
