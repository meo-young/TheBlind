#include "TBPossessableActor.h"
#include "PaperFlipbookComponent.h"
#include "Camera/TBCameraComponent.h"
#include "Camera/TBPlayerCameraManager.h"
#include "Character/Player/TBPlayerController.h"
#include "Components/SphereComponent.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ATBPossessableActor::ATBPossessableActor()
{
	// 상호작용 시 빙의할 카메라 컴포넌트를 생성합니다.
	{
		EntryCamera = CreateDefaultSubobject<UTBCameraComponent>(TEXT("EntryComponent"));
		EntryCamera->SetupAttachment(FlipbookComponent);
	}
	
	// 닿으면 스트리밍 레벨을 로드하는 CapsuleComponent를 생성합니다.
	{
		LevelStreamingTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("LevelStreamingTrigger"));
		LevelStreamingTrigger->SetupAttachment(FlipbookComponent);
		LevelStreamingTrigger->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnLevelStreamingTriggerBeginOverlap);
		LevelStreamingTrigger->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnLevelStreamingTriggerEndOverlap);
	}
}

bool ATBPossessableActor::Interact(ATBPlayerController& PC)
{
	if (!bIsStreamingLevelLoaded && !StreamingLevel.IsNull()) return false;
	
	Super::Interact(PC);

	ATBPlayerCameraManager* CameraManager = Cast<ATBPlayerCameraManager>(PC.PlayerCameraManager);
	if (!IsValid(CameraManager))
	{
		UE_LOG(LogTemp, Error, TEXT("스트리밍 카메라 전환 실패: TBPlayerCameraManager를 찾을 수 없습니다."));
		return false;
	}

	AActor* FinalCameraActor = StreamingCameraActor.Get();
	if (!IsValid(FinalCameraActor))
	{
		UE_LOG(LogTemp, Error, TEXT("스트리밍 카메라 전환 실패: StreamingCameraActor가 설정되지 않았습니다."));
		return false;
	}

	const UTBCameraComponent* FinalCamera = FinalCameraActor->FindComponentByClass<UTBCameraComponent>();
	if (!IsValid(FinalCamera))
	{
		UE_LOG(LogTemp, Error, TEXT("스트리밍 카메라 전환 실패: StreamingCameraActor에 TBCameraComponent가 없습니다."));
		return false;
	}

	FTBCameraStep EntryCameraStep;
	EntryCameraStep.ViewTarget = this;
	EntryCameraStep.TransitionParams = EntryCamera->ViewTargetTransitionParams;

	FTBCameraStep FinalCameraStep;
	FinalCameraStep.ViewTarget = FinalCameraActor;
	FinalCameraStep.TransitionParams = FinalCamera->ViewTargetTransitionParams;
	FinalCameraStep.CameraShakeClass = FinalCamera->CameraShakeClass;
	FinalCameraStep.CameraShakeScale = FinalCamera->CameraShakeScale;
	FinalCameraStep.bStopCameraShakeImmediately = FinalCamera->bStopCameraShakeImmediately;

	CameraManager->BeginCameraTransition(EntryCameraStep, FinalCameraStep);
	
	return true;
}

void ATBPossessableActor::OnLevelStreamingTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != UGameplayStatics::GetPlayerPawn(this, 0) || StreamingLevel.IsNull() || bShouldStreamingLevelBeLoaded)
	{
		return;
	}

	bShouldStreamingLevelBeLoaded = true;
	bIsStreamingLevelLoaded = false;

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = GET_FUNCTION_NAME_CHECKED(ThisClass, HandleStreamingLevelLoaded);
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = static_cast<int32>(HashCombine(GetUniqueID(), ++StreamingRequestSerial));

	// EntryCamera 보간과 동시에 비동기로 로드하고, 표시까지 끝난 뒤 콜백을 받습니다.
	UGameplayStatics::LoadStreamLevelBySoftObjectPtr(this, StreamingLevel, true, false, LatentInfo);
}

void ATBPossessableActor::OnLevelStreamingTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != UGameplayStatics::GetPlayerPawn(this, 0) || LevelStreamingTrigger->IsOverlappingActor(OtherActor))
	{
		return;
	}

	bShouldStreamingLevelBeLoaded = false;
	bIsStreamingLevelLoaded = false;

	if (StreamingLevel.IsNull())
	{
		return;
	}

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.UUID = static_cast<int32>(HashCombine(GetUniqueID(), ++StreamingRequestSerial));

	// 로드 요청과 다른 UUID를 사용하므로 로드 도중 빠져나가도 언로드 요청이 무시되지 않습니다.
	UGameplayStatics::UnloadStreamLevelBySoftObjectPtr(this, StreamingLevel, LatentInfo, false);
}

void ATBPossessableActor::HandleStreamingLevelLoaded()
{
	// 로드가 끝나기 전에 Trigger를 벗어났다면 뒤늦은 완료 콜백은 무시합니다.
	if (!bShouldStreamingLevelBeLoaded)
	{
		return;
	}

	const ULevelStreaming* LoadedStreamingLevel = UGameplayStatics::GetStreamingLevel(this, StreamingLevel.GetLongPackageFName());

	if (!IsValid(LoadedStreamingLevel) || !LoadedStreamingLevel->IsLevelLoaded() || !LoadedStreamingLevel->IsLevelVisible())
	{
		UE_LOG(LogTemp, Error, TEXT("스트리밍 레벨 로드 실패: %s"), *StreamingLevel.ToString());
		return;
	}
	
	bIsStreamingLevelLoaded = true;
}
