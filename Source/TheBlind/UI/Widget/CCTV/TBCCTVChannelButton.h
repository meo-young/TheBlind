#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "TBCCTVChannelButton.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FTBCCTVChannelSelected, int32);

UCLASS(meta = (DisplayName = "CCTV Channel Button"))
class THEBLIND_API UTBCCTVChannelButton : public UButton
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Initializer
// ─────────────────────────────────────────────────────────────
public:
	/** 버튼 클릭 이벤트와 채널 선택 이벤트를 연결합니다. */
	void InitializeChannelButton();

	/** 버튼 클릭 이벤트와 채널 선택 이벤트의 연결을 해제합니다. */
	void ShutdownChannelButton();



// ─────────────────────────────────────────────────────────────
// Delegate
// ─────────────────────────────────────────────────────────────
public:
	FTBCCTVChannelSelected& OnChannelSelected() { return ChannelSelectedEvent; }



// ─────────────────────────────────────────────────────────────
// Getter
// ─────────────────────────────────────────────────────────────
public:
	int32 GetChannelIndex() const { return ChannelIndex; }



// ─────────────────────────────────────────────────────────────
// Button Callback
// ─────────────────────────────────────────────────────────────
private:
	UFUNCTION()
	void HandleClicked();



// ─────────────────────────────────────────────────────────────
// Delegate
// ─────────────────────────────────────────────────────────────
private:
	/** 버튼 클릭 시 설정된 CCTV 채널 인덱스를 전달하는 이벤트입니다. */
	FTBCCTVChannelSelected ChannelSelectedEvent;



// ─────────────────────────────────────────────────────────────
// Channel
// ─────────────────────────────────────────────────────────────
private:
	/** 이 버튼이 선택할 CCTVCameras 배열의 인덱스입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|CCTV", meta = (ClampMin = "0"))
	int32 ChannelIndex = INDEX_NONE;
};
