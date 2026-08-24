#include "TBInteractableActor.h"
#include "PaperFlipbookComponent.h"
#include "Components/BoxComponent.h"

ATBInteractableActor::ATBInteractableActor()
{
	// FlipbookComponent를 생성합니다.
	{
		Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
		SetRootComponent(Mesh);
	}
	
	// 상호작용 영역을 인식하는 Collision을 생성합니다.
	{
		InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
		InteractionBox->SetupAttachment(Mesh);
	}
}

bool ATBInteractableActor::CanInteract() const
{
	return true;
}

bool ATBInteractableActor::Interact(ATBPlayerController& PC)
{
	UE_LOG(LogTemp, Warning, TEXT("상호작용 대상 : %s"), *GetNameSafe(this));
	return true;
}
