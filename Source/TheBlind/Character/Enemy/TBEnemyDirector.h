#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Location/TBLocationTypes.h"
#include "TimerManager.h"
#include "TBEnemyDirector.generated.h"

class ALevelSequenceActor;
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
// Enemy Location State
// ─────────────────────────────────────────────────────────────
public:
	/** 전달받은 Enemy가 Director에 등록되어 있는지 반환합니다. */
	bool IsEnemyRegistered(const AActor& Enemy) const;

	/** 전달받은 Enemy의 현재 장소를 OutLocation으로 반환합니다. */
	bool TryGetEnemyLocation(const AActor& Enemy, ETBLocation& OutLocation) const;

	/** 전달받은 Enemy가 현재 장소에서 이동할 수 있는 장소들을 반환합니다. */
	TArray<ETBLocation> GetReachableLocations(const AActor& Enemy) const;

	/** 전달받은 Enemy를 이동 가능한 장소로 변경합니다. */
	bool TryMoveEnemy(AActor& Enemy, ETBLocation Destination);


// ─────────────────────────────────────────────────────────────
// Enemy Move Timer
// ─────────────────────────────────────────────────────────────
private:
	/** 전달받은 Enemy 인덱스의 다음 장소 이동을 예약합니다. */
	void ScheduleEnemyMove(int32 EnemyIndex);

	/** 전달받은 Enemy 인덱스의 이동 가능 장소를 선택하고 현재 장소를 변경합니다. */
	void HandleEnemyMoveTimer(int32 EnemyIndex);


// ─────────────────────────────────────────────────────────────
// Enemy Sequence
// ─────────────────────────────────────────────────────────────
private:
	/** 전달받은 장소의 LevelSequenceActor를 처음부터 재생합니다. */
	bool PlayEnemyLocationSequence(FTBEnemyLocationState& EnemyState, ETBLocation Location);


// ─────────────────────────────────────────────────────────────
// Location Graph
// ─────────────────────────────────────────────────────────────
public:
	/** 전달받은 장소의 설정을 반환합니다. 등록되지 않은 장소라면 nullptr을 반환합니다. */
	const FTBLocationDefinition* FindLocation(ETBLocation Location) const;

	/** 출발 장소에서 도착 장소로 직접 이동할 수 있는지 반환합니다. */
	bool IsMoveAllowed(ETBLocation From, ETBLocation To) const;

	/** 전달받은 장소에서 직접 이동할 수 있는 장소들을 반환합니다. */
	TArray<ETBLocation> GetReachableLocations(ETBLocation Location) const;


// ─────────────────────────────────────────────────────────────
// Enemy State Lookup
// ─────────────────────────────────────────────────────────────
private:
	/** 전달받은 Enemy의 장소 상태를 반환합니다. 등록되지 않았다면 nullptr을 반환합니다. */
	FTBEnemyLocationState* FindEnemyState(const AActor& Enemy);

	/** 전달받은 Enemy의 읽기 전용 장소 상태를 반환합니다. 등록되지 않았다면 nullptr을 반환합니다. */
	const FTBEnemyLocationState* FindEnemyState(const AActor& Enemy) const;


// ─────────────────────────────────────────────────────────────
// Configuration
// ─────────────────────────────────────────────────────────────
protected:
	/** 모든 Enemy가 공유하는 장소 이동 그래프입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|장소")
	TMap<ETBLocation, FTBLocationDefinition> LocationGraph;

	/** Enemy 이동 노이즈를 표시할 CCTV Monitor입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|CCTV")
	TObjectPtr<ATBMonitor> CCTVMonitor;

	/** 장소 상태를 각각 관리할 Enemy 목록입니다. */
	UPROPERTY(EditInstanceOnly, Category = "변수|Enemy")
	TArray<FTBEnemyLocationState> EnemyStates;
};
