#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Location/TBLocationTypes.h"
#include "TBCCTVChannelButton.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FTBCCTVChannelSelected, ETBLocation);

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
	/** CCTV 장소 선택 이벤트를 반환합니다. */
	FTBCCTVChannelSelected& OnChannelSelected() { return ChannelSelectedEvent; }


// ─────────────────────────────────────────────────────────────
// Getter
// ─────────────────────────────────────────────────────────────
public:
	/** 이 버튼이 선택할 CCTV 장소를 반환합니다. */
	ETBLocation GetTargetLocation() const { return TargetLocation; }


// ─────────────────────────────────────────────────────────────
// Button Callback
// ─────────────────────────────────────────────────────────────
private:
	/** 설정된 CCTV 장소를 선택 이벤트로 전달합니다. */
	UFUNCTION()
	void HandleClicked();


// ─────────────────────────────────────────────────────────────
// Delegate
// ─────────────────────────────────────────────────────────────
private:
	/** 버튼 클릭 시 설정된 CCTV 장소를 전달하는 이벤트입니다. */
	FTBCCTVChannelSelected ChannelSelectedEvent;


// ─────────────────────────────────────────────────────────────
// Channel
// ─────────────────────────────────────────────────────────────
private:
	/** 이 버튼이 선택할 CCTV 장소입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|CCTV")
	ETBLocation TargetLocation = ETBLocation::Dining;
};
