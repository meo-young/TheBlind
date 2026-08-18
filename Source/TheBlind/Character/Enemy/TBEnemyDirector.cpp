#include "TBEnemyDirector.h"
#include "Actor/TBMonitor.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

void ATBEnemyDirector::BeginPlay()
{
	Super::BeginPlay();

	// Enemy마다 시작 장소의 시퀀스를 재생하고 독립적인 이동 타이머를 예약합니다.
	for (int32 EnemyIndex = 0; EnemyIndex < EnemyStates.Num(); ++EnemyIndex)
	{
		FTBEnemyLocationState& EnemyState = EnemyStates[EnemyIndex];
		EnemyState.CurrentLocation = EnemyState.StartLocation;

		if (!EnemyState.Enemy)
		{
			UE_LOG(LogTemp, Error, TEXT("Enemy 이동 초기화 실패: EnemyStates[%d]에 Enemy가 설정되지 않았습니다."), EnemyIndex);
			continue;
		}

		PlayEnemyLocationSequence(EnemyState, EnemyState.StartLocation);
		ScheduleEnemyMove(EnemyIndex);
	}
}

bool ATBEnemyDirector::IsEnemyRegistered(const AActor& Enemy) const
{
	return FindEnemyState(Enemy) != nullptr;
}

bool ATBEnemyDirector::TryGetEnemyLocation(const AActor& Enemy, ETBLocation& OutLocation) const
{
	const FTBEnemyLocationState* EnemyState = FindEnemyState(Enemy);
	if (!EnemyState)
	{
		return false;
	}

	OutLocation = EnemyState->CurrentLocation;
	return true;
}

TArray<ETBLocation> ATBEnemyDirector::GetReachableLocations(const AActor& Enemy) const
{
	const FTBEnemyLocationState* EnemyState = FindEnemyState(Enemy);
	if (!EnemyState)
	{
		return {};
	}

	return GetReachableLocations(EnemyState->CurrentLocation);
}

bool ATBEnemyDirector::TryMoveEnemy(AActor& Enemy, const ETBLocation Destination)
{
	FTBEnemyLocationState* EnemyState = FindEnemyState(Enemy);
	if (!EnemyState)
	{
		return false;
	}

	// 장소 연결과 시퀀스 구성이 모두 유효한 경우에만 현재 장소를 변경합니다.
	if (!IsMoveAllowed(EnemyState->CurrentLocation, Destination))
	{
		return false;
	}

	const ETBLocation PreviousLocation = EnemyState->CurrentLocation;
	if (!PlayEnemyLocationSequence(*EnemyState, Destination))
	{
		return false;
	}

	EnemyState->CurrentLocation = Destination;
	if (CCTVMonitor)
	{
		CCTVMonitor->ShowEnemyMovementNoise(PreviousLocation);
	}
	return true;
}

void ATBEnemyDirector::ScheduleEnemyMove(const int32 EnemyIndex)
{
	FTBEnemyLocationState& EnemyState = EnemyStates[EnemyIndex];

	// 최대 시간이 최소 시간보다 작으면 최소 시간을 사용합니다.
	const float MinMoveInterval = FMath::Max(EnemyState.MinMoveInterval, 0.1f);
	const float MaxMoveInterval = FMath::Max(EnemyState.MaxMoveInterval, MinMoveInterval);
	const float MoveInterval = FMath::FRandRange(MinMoveInterval, MaxMoveInterval);

	FTimerDelegate MoveTimerDelegate;
	MoveTimerDelegate.BindUObject(this, &ThisClass::HandleEnemyMoveTimer, EnemyIndex);
	GetWorldTimerManager().SetTimer(EnemyState.MoveTimerHandle, MoveTimerDelegate, MoveInterval, false);
}

void ATBEnemyDirector::HandleEnemyMoveTimer(const int32 EnemyIndex)
{
	FTBEnemyLocationState& EnemyState = EnemyStates[EnemyIndex];

	// 현재 장소와 직접 연결된 장소 중 하나를 무작위로 선택합니다.
	const TArray<ETBLocation> ReachableLocations = GetReachableLocations(EnemyState.CurrentLocation);
	if (ReachableLocations.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy 이동 중단: %s에서 이동할 수 있는 장소가 없습니다."), *UEnum::GetValueAsString(EnemyState.CurrentLocation));
		return;
	}

	const ETBLocation PreviousLocation = EnemyState.CurrentLocation;
	const int32 DestinationIndex = FMath::RandRange(0, ReachableLocations.Num() - 1);
	const ETBLocation Destination = ReachableLocations[DestinationIndex];

	if (TryMoveEnemy(*EnemyState.Enemy, Destination))
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy 이동: %s, %s -> %s"), *GetNameSafe(EnemyState.Enemy), *UEnum::GetValueAsString(PreviousLocation), *UEnum::GetValueAsString(Destination));
	}

	// 이동 성공 여부와 관계없이 다음 이동 시점을 다시 예약합니다.
	ScheduleEnemyMove(EnemyIndex);
}

bool ATBEnemyDirector::PlayEnemyLocationSequence(FTBEnemyLocationState& EnemyState, const ETBLocation Location)
{
	const FTBLocationSequenceSet* SequenceSet = EnemyState.LocationSequences.Find(Location);
	if (!SequenceSet || SequenceSet->SequenceActors.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy 시퀀스 재생 실패: %s의 %s 장소에 LevelSequenceActor 목록이 설정되지 않았습니다."), *GetNameSafe(EnemyState.Enemy), *UEnum::GetValueAsString(Location));
		return false;
	}

	// 비어 있거나 제거된 항목을 제외한 뒤 이번 장소에서 재생할 시퀀스를 무작위로 선택합니다.
	TArray<ALevelSequenceActor*> ValidSequenceActors;
	for (const TObjectPtr<ALevelSequenceActor>& SequenceActor : SequenceSet->SequenceActors)
	{
		if (IsValid(SequenceActor))
		{
			ValidSequenceActors.Add(SequenceActor);
		}
	}

	if (ValidSequenceActors.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy 시퀀스 재생 실패: %s의 %s 장소에 유효한 LevelSequenceActor가 없습니다."), *GetNameSafe(EnemyState.Enemy), *UEnum::GetValueAsString(Location));
		return false;
	}

	const int32 SequenceIndex = FMath::RandRange(0, ValidSequenceActors.Num() - 1);
	ALevelSequenceActor* SequenceActor = ValidSequenceActors[SequenceIndex];
	ULevelSequencePlayer* SequencePlayer = SequenceActor->GetSequencePlayer();
	if (!SequencePlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy 시퀀스 재생 실패: %s에 Level Sequence Asset이 설정되지 않았습니다."), *GetNameSafe(SequenceActor));
		return false;
	}

	// 이전 장소의 시퀀스를 정지한 뒤 새 시퀀스를 처음부터 재생합니다.
	if (EnemyState.ActiveSequenceActor.IsValid())
	{
		ULevelSequencePlayer* ActiveSequencePlayer = EnemyState.ActiveSequenceActor->GetSequencePlayer();
		if (ActiveSequencePlayer)
		{
			ActiveSequencePlayer->Stop();
		}
	}

	SequencePlayer->StopAtCurrentTime();
	SequencePlayer->SetPlaybackPosition(FMovieSceneSequencePlaybackParams(SequencePlayer->GetStartTime().Time, EUpdatePositionMethod::Jump));
	SequencePlayer->Play();
	EnemyState.ActiveSequenceActor = SequenceActor;
	return true;
}

const FTBLocationDefinition* ATBEnemyDirector::FindLocation(const ETBLocation Location) const
{
	return LocationGraph.Find(Location);
}

bool ATBEnemyDirector::IsMoveAllowed(const ETBLocation From, const ETBLocation To) const
{
	const FTBLocationDefinition* LocationDefinition = FindLocation(From);
	if (!LocationDefinition)
	{
		return false;
	}

	return LocationDefinition->ReachableLocations.Contains(To);
}

TArray<ETBLocation> ATBEnemyDirector::GetReachableLocations(const ETBLocation Location) const
{
	const FTBLocationDefinition* LocationDefinition = FindLocation(Location);
	if (!LocationDefinition)
	{
		return {};
	}

	return LocationDefinition->ReachableLocations;
}

FTBEnemyLocationState* ATBEnemyDirector::FindEnemyState(const AActor& Enemy)
{
	for (FTBEnemyLocationState& EnemyState : EnemyStates)
	{
		if (EnemyState.Enemy == &Enemy)
		{
			return &EnemyState;
		}
	}

	return nullptr;
}

const FTBEnemyLocationState* ATBEnemyDirector::FindEnemyState(const AActor& Enemy) const
{
	for (const FTBEnemyLocationState& EnemyState : EnemyStates)
	{
		if (EnemyState.Enemy == &Enemy)
		{
			return &EnemyState;
		}
	}

	return nullptr;
}
