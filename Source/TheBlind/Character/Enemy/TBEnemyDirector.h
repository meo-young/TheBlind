#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Location/TBLocationTypes.h"
#include "TimerManager.h"
#include "TBEnemyDirector.generated.h"

class ALevelSequenceActor;
class ACameraActor;
class ATBMonitor;

/** 한 장소에서 선택할 수 있는 LevelSequenceActor 목록을 관리합니다. */
USTRUCT()
struct FTBLocationSequenceSet
{
	GENERATED_BODY()

	/** 이 장소에서 Enemy를 연출할 LevelSequenceActor 후보 목록입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|시퀀스")
	TArray<TObjectPtr<ALevelSequenceActor>> SequenceActors;
};

/** Enemy 하나의 시작 장소와 현재 장소를 관리합니다. */
USTRUCT()
struct FTBEnemyLocationState
{
	GENERATED_BODY()

	/** 장소 상태를 관리할 Enemy입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|Enemy")
	TObjectPtr<AActor> Enemy;

	/** 게임 시작 시 Enemy가 위치할 장소입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|장소")
	ETBLocation StartLocation = ETBLocation::Dining;

	/** 게임 실행 중 Enemy가 위치한 현재 장소입니다. */
	ETBLocation CurrentLocation = ETBLocation::Dining;

	/** 장소마다 Enemy를 연출할 LevelSequenceActor 후보 목록입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|시퀀스")
	TMap<ETBLocation, FTBLocationSequenceSet> LocationSequences;

	/** 현재 Enemy를 연출하고 있는 LevelSequenceActor입니다. */
	TWeakObjectPtr<ALevelSequenceActor> ActiveSequenceActor;

	/** Enemy가 다음 장소로 이동하기까지의 최소 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|이동", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float MinMoveInterval = 8.0f;

	/** Enemy가 다음 장소로 이동하기까지의 최대 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|이동", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float MaxMoveInterval = 15.0f;

	/** Enemy의 다음 장소 이동을 예약하는 타이머입니다. */
	FTimerHandle MoveTimerHandle;

	/** Enemy가 위협 장소에 진입한 뒤 플레이어가 사망하기까지의 시간입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|사망", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float PlayerDeathDelay = 5.0f;

	/** 이 Enemy가 플레이어를 사망시켰을 때 재생할 LevelSequenceActor입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|사망")
	TObjectPtr<ALevelSequenceActor> DeathSequenceActor;

	/** Enemy의 플레이어 사망 처리를 예약하는 타이머입니다. */
	FTimerHandle PlayerDeathTimerHandle;

	/** Enemy가 다시 나타나는 시점을 예약하는 타이머입니다. */
	FTimerHandle RelocationTimerHandle;
};

UCLASS()
class THEBLIND_API ATBEnemyDirector : public AActor
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────
public:
	/** 등록된 Enemy들의 현재 장소를 시작 장소로 초기화합니다. */
	virtual void BeginPlay() override;


// ─────────────────────────────────────────────────────────────
// Enemy Relocation
// ─────────────────────────────────────────────────────────────
public:
	/** 지정 장소의 Enemy 재배치를 시작하고 성공 여부를 반환합니다. */
	bool TryBeginEnemyRelocation(ETBLocation SourceLocation);

private:
	/** 지정한 Enemy의 재배치 대기 시간을 끝내고 무작위 장소에서 다시 나타나게 합니다. */
	void HandleEnemyRelocationTimer(int32 EnemyIndex);


// ─────────────────────────────────────────────────────────────
// Enemy Move Timer
// ─────────────────────────────────────────────────────────────
private:
	/** 전달받은 Enemy를 선택된 장소로 이동합니다. */
	bool TryMoveEnemy(AActor& Enemy, ETBLocation Destination);

	/** 전달받은 Enemy 인덱스의 다음 장소 이동을 예약합니다. */
	void ScheduleEnemyMove(int32 EnemyIndex);

	/** 전달받은 Enemy 인덱스의 이동 가능 장소를 선택하고 현재 장소를 변경합니다. */
	void HandleEnemyMoveTimer(int32 EnemyIndex);

	/** 모든 Enemy의 이동 타이머와 현재 장소 시퀀스를 중지합니다. */
	void StopAllEnemyMovement();


// ─────────────────────────────────────────────────────────────
// Player Death Timer
// ─────────────────────────────────────────────────────────────
private:
	/** 전달받은 장소가 플레이어 사망 카운트다운을 시작하는 장소인지 반환합니다. */
	bool IsPlayerDeathLocation(ETBLocation Location) const;

	/** Enemy의 현재 장소에 따라 플레이어 사망 타이머를 갱신합니다. */
	void UpdatePlayerDeathTimer(FTBEnemyLocationState& EnemyState);

	/** 사망 시간이 경과한 Enemy를 확인하고 플레이어 사망을 처리합니다. */
	void HandlePlayerDeathTimer(AActor* Enemy);


// ─────────────────────────────────────────────────────────────
// Enemy Sequence
// ─────────────────────────────────────────────────────────────
private:
	/** 전달받은 장소의 LevelSequenceActor를 재생합니다. */
	bool PlayEnemyLocationSequence(FTBEnemyLocationState& EnemyState, ETBLocation Location);

	/** 전달받은 Enemy의 사망 연출 LevelSequenceActor를 재생합니다. */
	bool PlayEnemyDeathSequence(FTBEnemyLocationState& EnemyState);


// ─────────────────────────────────────────────────────────────
// Enemy State Lookup
// ─────────────────────────────────────────────────────────────
private:
	/** 전달받은 Enemy의 장소 상태를 반환합니다. 등록되지 않았다면 nullptr을 반환합니다. */
	FTBEnemyLocationState* FindEnemyState(const AActor& Enemy);


// ─────────────────────────────────────────────────────────────
// Configuration
// ─────────────────────────────────────────────────────────────
protected:
	/** 모든 Enemy가 공유하는 장소 이동 그래프입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|장소")
	TMap<ETBLocation, FTBLocationDefinition> LocationGraph;

	/** 기믹으로 숨겨진 모든 Enemy가 공통으로 사용할 재등장 장소 후보입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|재배치")
	TArray<ETBLocation> EnemyRespawnLocations;

	/** Enemy가 모습을 감추고 재등장하기까지의 공통 시간입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|재배치", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float EnemyHiddenDuration = 10.0f;

	/** Enemy 이동 노이즈를 표시할 CCTV Monitor입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|CCTV")
	TObjectPtr<ATBMonitor> CCTVMonitor;

	/** 플레이어 사망 연출을 촬영할 맵의 카메라입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|사망")
	TObjectPtr<ACameraActor> PlayerDeathCamera;

	/** 장소 상태를 각각 관리할 Enemy 목록입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|Enemy")
	TArray<FTBEnemyLocationState> EnemyStates;
};
