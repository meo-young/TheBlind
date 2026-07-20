#include "TBInteractableActor.h"
#include "PaperFlipbookComponent.h"
#include "Components/BoxComponent.h"

ATBInteractableActor::ATBInteractableActor()
{
	// FlipbookComponent를 생성합니다.
	{
		FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Flipbook"));
		SetRootComponent(FlipbookComponent);
	}
	
	// 상호작용 영역을 인식하는 Collision을 생성합니다.
	{
		InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
		InteractionBox->SetupAttachment(FlipbookComponent);
	}
}

bool ATBInteractableActor::Interact(ATBPlayerController& PC)
{
	UE_LOG(LogTemp, Warning, TEXT("상호작용 대상 : %s"), *GetNameSafe(this));
	return true;
}