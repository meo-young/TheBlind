#include "TBHUD.h"
#include "Character/Player/TBPlayerController.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Interact/Interactable.h"
#include "UObject/ConstructorHelpers.h"

ATBHUD::ATBHUD()
{
	// 준비된 3종 크로스헤어 텍스처를 상태별 기본값으로 설정합니다.
	{
		static ConstructorHelpers::FObjectFinder<UTexture2D> DefaultCrosshair(TEXT("/Game/_TheBlind/Texture/Crosshair/T_Crosshair_Normal.T_Crosshair_Normal"));
		static ConstructorHelpers::FObjectFinder<UTexture2D> InteractableCrosshair(TEXT("/Game/_TheBlind/Texture/Crosshair/T_Crosshair_Interact.T_Crosshair_Interact"));
		static ConstructorHelpers::FObjectFinder<UTexture2D> UnavailableCrosshair(TEXT("/Game/_TheBlind/Texture/Crosshair/T_Crosshair_Pause.T_Crosshair_Pause"));
		DefaultCrosshairTexture = DefaultCrosshair.Object;
		InteractableCrosshairTexture = InteractableCrosshair.Object;
		UnavailableCrosshairTexture = UnavailableCrosshair.Object;
	}
}

void ATBHUD::DrawHUD()
{
	Super::DrawHUD();

	ATBPlayerController* PC = CastChecked<ATBPlayerController>(PlayerOwner);
	UTexture2D* CrosshairTexture = DefaultCrosshairTexture;

	// 카메라 중앙에 상호작용 대상이 있으면 현재 가능 여부에 맞는 텍스처를 선택합니다.
	if (IInteractable* Interactable = PC->FindInteractionTarget())
	{
		if (Interactable->CanInteract())
		{
			CrosshairTexture = InteractableCrosshairTexture;
		}
		else
		{
			CrosshairTexture = UnavailableCrosshairTexture;
		}
	}

	// 선택한 크로스헤어를 원본 비율로 화면 정중앙에 배치합니다.
	const float CrosshairWidth = CrosshairTexture->GetSizeX() * CrosshairScale;
	const float CrosshairHeight = CrosshairTexture->GetSizeY() * CrosshairScale;
	const float DrawX = (Canvas->ClipX - CrosshairWidth) * 0.5f;
	const float DrawY = (Canvas->ClipY - CrosshairHeight) * 0.5f;
	DrawTexture(CrosshairTexture, DrawX, DrawY, CrosshairWidth, CrosshairHeight, 0.0f, 0.0f, 1.0f, 1.0f);
}
