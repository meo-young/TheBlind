#include "TBPossessableActor.h"
#include "PaperFlipbookComponent.h"
#include "Camera/TBCameraComponent.h"
#include "Character/Player/TBPlayerController.h"

ATBPossessableActor::ATBPossessableActor()
{
	// 상호작용 시 빙의할 카메라 컴포넌트를 생성합니다.
	{
		EntryCamera = CreateDefaultSubobject<UTBCameraComponent>(TEXT("EntryComponent"));
		EntryCamera->SetupAttachment(FlipbookComponent);
	}
}

void ATBPossessableActor::Interact(ATBPlayerController& PC)
{
	Super::Interact(PC);
	
	PC.BeginRemoteSequence(*this);
}