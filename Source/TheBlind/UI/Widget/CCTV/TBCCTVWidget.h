#pragma once

#include "CoreMinimal.h"
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
	virtual void NativeConstruct() override;
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
	void HandleChannelSelected(int32 ChannelIndex);



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
