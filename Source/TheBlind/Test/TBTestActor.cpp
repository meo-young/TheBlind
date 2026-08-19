#include "TBTestActor.h"

#if WITH_EDITOR
#include "Camera/CameraActor.h"
#include "Camera/TBPlayerCameraManager.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#endif

void ATBTestActor::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	if (!bIsLevelSequenceCameraTestEnabled)
	{
		return;
	}

	RunLevelSequenceCameraTest();
#endif
}

bool ATBTestActor::IsEditorOnly() const
{
	return true;
}

#if WITH_EDITOR
void ATBTestActor::RunLevelSequenceCameraTest()
{
	// 테스트에 필요한 맵 배치 Actor가 모두 설정되었는지 확인합니다.
	if (!LevelSequenceTestCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("LevelSequence 카메라 테스트 실패: %s에 LevelSequenceTestCamera가 설정되지 않았습니다."), *GetNameSafe(this));
		return;
	}

	if (!LevelSequenceTestActor)
	{
		UE_LOG(LogTemp, Error, TEXT("LevelSequence 카메라 테스트 실패: %s에 LevelSequenceTestActor가 설정되지 않았습니다."), *GetNameSafe(this));
		return;
	}

	ULevelSequencePlayer* SequencePlayer = LevelSequenceTestActor->GetSequencePlayer();
	if (!SequencePlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("LevelSequence 카메라 테스트 실패: %s에 Level Sequence Asset이 설정되지 않았습니다."), *GetNameSafe(LevelSequenceTestActor));
		return;
	}

	// 진행 중인 카메라 전환을 정리한 뒤 테스트 카메라로 즉시 전환합니다.
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	ATBPlayerCameraManager* CameraManager = CastChecked<ATBPlayerCameraManager>(PlayerController->PlayerCameraManager);
	CameraManager->CutToViewTarget(*LevelSequenceTestCamera);

	// 테스트 LevelSequence를 처음부터 재생합니다.
	SequencePlayer->StopAtCurrentTime();
	SequencePlayer->SetPlaybackPosition(FMovieSceneSequencePlaybackParams(SequencePlayer->GetStartTime().Time, EUpdatePositionMethod::Jump));
	SequencePlayer->Play();
}
#endif
