#pragma once

#include "CoreMinimal.h"
#include "Location/TBLocationTypes.h"
#include "UI/Widget/TBUserWidget.h"
#include "TBCCTVWidget.generated.h"

class ATBMonitor;
class UTBCCTVChannelButton;

UCLASS(Abstract)
class THEBLIND_API UTBCCTVWidget : public UTBUserWidget
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// UserWidget Interface
// ─────────────────────────────────────────────────────────────
protected:
	/** CCTV 버튼을 찾아 장소 선택 이벤트를 연결합니다. */
	virtual void NativeConstruct() override;

	/** CCTV 버튼의 장소 선택 이벤트 연결을 해제합니다. */
	virtual void NativeDestruct() override;


// ─────────────────────────────────────────────────────────────
// Initializer
// ─────────────────────────────────────────────────────────────
public:
	/** CCTV 선택 요청을 전달할 모니터를 설정합니다. */
	void SetMonitor(ATBMonitor* InMonitor);


// ─────────────────────────────────────────────────────────────
// Channel Callback
// ─────────────────────────────────────────────────────────────
private:
	/** 선택한 장소의 CCTV 전환을 Monitor에 요청합니다. */
	void HandleChannelSelected(ETBLocation Location);


// ─────────────────────────────────────────────────────────────
// Reference
// ─────────────────────────────────────────────────────────────
private:
	/** CCTV 선택 요청을 처리하는 현재 모니터입니다. */
	UPROPERTY(Transient)
	TObjectPtr<ATBMonitor> Monitor;


// ─────────────────────────────────────────────────────────────
// Cache Variable
// ─────────────────────────────────────────────────────────────
private:
	/** WidgetTree에서 찾아 이벤트를 연결한 CCTV 채널 버튼 모음입니다. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTBCCTVChannelButton>> ChannelButtons;
	
};
