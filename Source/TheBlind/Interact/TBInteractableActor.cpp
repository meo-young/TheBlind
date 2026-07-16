#include "TBInteractableActor.h"
#include "PaperFlipbookComponent.h"
#include "Components/BoxComponent.h"

ATBInteractableActor::ATBInteractableActor()
{
	// SceneComponent를 생성합니다.
	{
		SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
		SetRootComponent(SceneComponent);
	}
	
	// FlipbookComponent를 생성합니다.
	{
		FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Flipbook"));
		FlipbookComponent->SetupAttachment(SceneComponent);
	}
	
	// 상호작용 영역을 인식하는 Collision을 생성합니다.
	{
		InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
		InteractionBox->SetupAttachment(FlipbookComponent);
	}
}

void ATBInteractableActor::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("상호작용 대상 : %s"), *GetNameSafe(this));
}
