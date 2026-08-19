#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/TBEnemyDirector.h"
#include "Interact/TBInteractableActor.h"
#include "TBEnemyGimmickDevice.generated.h"

class ALevelSequenceActor;

UCLASS()
class THEBLIND_API ATBEnemyGimmickDevice : public ATBInteractableActor
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Interactable Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 대상 장소의 Enemy를 확인하고 숨김과 재배치를 요청합니다. */
	virtual bool Interact(ATBPlayerController& PC) override;


// ─────────────────────────────────────────────────────────────
// Gimmick Configuration
// ─────────────────────────────────────────────────────────────
protected:
	/** Enemy의 장소와 이동 상태를 관리하는 Director입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|기믹")
	TObjectPtr<ATBEnemyDirector> EnemyDirector;

	/** 상호작용 시 Enemy가 있는지 확인할 장소입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|기믹")
	ETBLocation TargetLocation = ETBLocation::PlayerRoomApproach1;

	/** Enemy가 발견된 경우 재생할 장치별 성공 연출입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|연출")
	TObjectPtr<ALevelSequenceActor> SuccessSequenceActor;

	/** Enemy가 없는 경우 재생할 장치별 실패 또는 페널티 연출입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|연출")
	TObjectPtr<ALevelSequenceActor> FailureSequenceActor;
};
