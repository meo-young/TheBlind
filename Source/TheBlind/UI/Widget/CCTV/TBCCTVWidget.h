#pragma once

#include "CoreMinimal.h"
#include "Location/TBLocationTypes.h"
#include "UI/Widget/TBUserWidget.h"
#include "TBCCTVWidget.generated.h"

class ATBMonitor;
class UTBCCTVChannelButton;
class UCanvasPanelSlot;
class UImage;
class UTexture2D;

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

	/** 현재 CCTV Widget을 소유하는 모니터의 월드 위치를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "CCTV")
	FVector GetMonitorLocation() const;

	/** 지정한 텍스처와 크기로 CCTV 화면의 가상 커서를 생성합니다. */
	void InitializeCCTVCursor(UTexture2D& CursorTexture, const FVector2D& CursorSize);

	/** CCTV 화면 안에서 가상 커서의 위치를 변경합니다. */
	void SetCCTVCursorPosition(const FVector2D& CursorPosition);

	/** CCTV 가상 커서의 표시 여부를 설정합니다. */
	void SetCCTVCursorVisible(bool bVisible);


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

	/** 런타임에 생성한 CCTV 가상 커서 이미지입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UImage> CCTVCursorImage;

	/** 가상 커서의 위치와 크기를 제어하는 Canvas Slot입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanelSlot> CCTVCursorSlot;
	
};
