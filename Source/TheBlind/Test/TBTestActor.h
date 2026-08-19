#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TBTestActor.generated.h"

class ACameraActor;
class ALevelSequenceActor;

UCLASS()
class THEBLIND_API ATBTestActor : public AActor
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 게임 시작 시 등록된 테스트 기능을 실행합니다. */
	virtual void BeginPlay() override;

	/** 이 Actor가 에디터 전용임을 반환합니다. */
	virtual bool IsEditorOnly() const override;


// ─────────────────────────────────────────────────────────────
// Level Sequence Camera Test
// ─────────────────────────────────────────────────────────────
#if WITH_EDITOR
private:
	/** 테스트 카메라로 즉시 전환한 뒤 LevelSequence를 처음부터 재생합니다. */
	void RunLevelSequenceCameraTest();
#endif


// ─────────────────────────────────────────────────────────────
// Level Sequence Camera Test Configuration
// ─────────────────────────────────────────────────────────────
#if WITH_EDITORONLY_DATA
protected:
	/** 활성화하면 게임 시작 시 LevelSequence 카메라 테스트를 실행합니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|카메라 테스트")
	uint8 bIsLevelSequenceCameraTestEnabled : 1 = false;

	/** 테스트 시작 시 ViewTarget으로 사용할 맵의 카메라입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|카메라 테스트")
	TObjectPtr<ACameraActor> LevelSequenceTestCamera;

	/** 테스트 시작 시 처음부터 재생할 맵의 LevelSequenceActor입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|카메라 테스트")
	TObjectPtr<ALevelSequenceActor> LevelSequenceTestActor;
#endif
};
