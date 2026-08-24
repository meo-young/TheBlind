#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TBHUD.generated.h"

class UTexture2D;

UCLASS()
class THEBLIND_API ATBHUD : public AHUD
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// HUD Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 크로스헤어 텍스처의 기본값을 설정합니다. */
	ATBHUD();

	/** 카메라 중앙의 상호작용 상태에 맞는 크로스헤어를 화면 중앙에 그립니다. */
	virtual void DrawHUD() override;


// ─────────────────────────────────────────────────────────────
// Crosshair Configuration
// ─────────────────────────────────────────────────────────────
protected:
	/** 상호작용 대상이 없을 때 표시할 기본 크로스헤어입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|크로스헤어")
	TObjectPtr<UTexture2D> DefaultCrosshairTexture;

	/** 현재 상호작용할 수 있는 대상에 표시할 크로스헤어입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|크로스헤어")
	TObjectPtr<UTexture2D> InteractableCrosshairTexture;

	/** 상호작용 대상이지만 현재 상호작용할 수 없을 때 표시할 크로스헤어입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|크로스헤어")
	TObjectPtr<UTexture2D> UnavailableCrosshairTexture;

	/** 원본 텍스처 크기에 적용할 화면 표시 배율입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|크로스헤어", meta = (ClampMin = "0.1"))
	float CrosshairScale = 1.0f;
};
