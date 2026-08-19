#include "TBEnemyDirector.h"
#include "Actor/TBMonitor.h"
#include "Core/TBGameMode.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

void ATBEnemyDirector::BeginPlay()
{
	Super::BeginPlay();

	if (!CCTVMonitor)
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy 이동 노이즈 초기화 실패: %s에 CCTVMonitor가 설정되지 않았습니다."), *GetNameSafe(this));
	}

	if (!PlayerDeathCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("플레이어 사망 연출 초기화 실패: %s에 PlayerDeathCamera가 설정되지 않았습니다."), *GetNameSafe(this));
	}

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

		if (!EnemyState.DeathSequenceActor)
		{
			UE_LOG(LogTemp, Error, TEXT("Enemy 사망 연출 초기화 실패: %s에 DeathSequenceActor가 설정되지 않았습니다."), *GetNameSafe(EnemyState.Enemy));
		}

		PlayEnemyLocationSequence(EnemyState, EnemyState.StartLocation);
		UpdatePlayerDeathTimer(EnemyState);
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
	if (bEnemyMovementStopped)
	{
		return false;
	}

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
	UpdatePlayerDeathTimer(*EnemyState);
	if (CCTVMonitor)
	{
		// 시청 중인 장소에서 Enemy가 나가거나 들어오는 경우 모두 이동 노이즈를 표시합니다.
		CCTVMonitor->ShowEnemyMovementNoise(PreviousLocation);
		CCTVMonitor->ShowEnemyMovementNoise(Destination);
	}
	return true;
}

void ATBEnemyDirector::ScheduleEnemyMove(const int32 EnemyIndex)
{
	if (bEnemyMovementStopped)
	{
		return;
	}

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
	if (bEnemyMovementStopped)
	{
		return;
	}

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

void ATBEnemyDirector::StopAllEnemyMovement()
{
	if (bEnemyMovementStopped)
	{
		return;
	}

	bEnemyMovementStopped = true;

	// 예약된 이동과 사망 처리를 취소하고 현재 재생 중인 장소 시퀀스를 모두 중지합니다.
	for (FTBEnemyLocationState& EnemyState : EnemyStates)
	{
		GetWorldTimerManager().ClearTimer(EnemyState.MoveTimerHandle);
		GetWorldTimerManager().ClearTimer(EnemyState.PlayerDeathTimerHandle);

		if (EnemyState.ActiveSequenceActor.IsValid())
		{
			ULevelSequencePlayer* SequencePlayer = EnemyState.ActiveSequenceActor->GetSequencePlayer();
			if (SequencePlayer)
			{
				SequencePlayer->Stop();
			}
			EnemyState.ActiveSequenceActor.Reset();
		}
	}
}

bool ATBEnemyDirector::IsPlayerDeathLocation(const ETBLocation Location) const
{
	return Location == ETBLocation::PlayerRoomApproach1 || Location == ETBLocation::PlayerRoomApproach2;
}

void ATBEnemyDirector::UpdatePlayerDeathTimer(FTBEnemyLocationState& EnemyState)
{
	if (!IsPlayerDeathLocation(EnemyState.CurrentLocation))
	{
		GetWorldTimerManager().ClearTimer(EnemyState.PlayerDeathTimerHandle);
		return;
	}

	// 두 위협 장소 사이에서 이동해도 기존 카운트다운을 유지합니다.
	if (GetWorldTimerManager().IsTimerActive(EnemyState.PlayerDeathTimerHandle))
	{
		return;
	}

	FTimerDelegate DeathTimerDelegate;
	DeathTimerDelegate.BindUObject(this, &ThisClass::HandlePlayerDeathTimer, EnemyState.Enemy.Get());
	GetWorldTimerManager().SetTimer(EnemyState.PlayerDeathTimerHandle, DeathTimerDelegate, EnemyState.PlayerDeathDelay, false);
}

void ATBEnemyDirector::HandlePlayerDeathTimer(AActor* Enemy)
{
	FTBEnemyLocationState* EnemyState = FindEnemyState(*Enemy);
	if (!EnemyState || !IsPlayerDeathLocation(EnemyState->CurrentLocation))
	{
		return;
	}

	ATBGameMode* GameMode = CastChecked<ATBGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode->HandlePlayerDeath(*Enemy, PlayerDeathCamera))
	{
		return;
	}

	StopAllEnemyMovement();
	PlayEnemyDeathSequence(*EnemyState);
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

bool ATBEnemyDirector::PlayEnemyDeathSequence(FTBEnemyLocationState& EnemyState)
{
	if (!EnemyState.DeathSequenceActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy 사망 시퀀스 재생 실패: %s에 DeathSequenceActor가 설정되지 않았습니다."), *GetNameSafe(EnemyState.Enemy));
		return false;
	}

	ULevelSequencePlayer* SequencePlayer = EnemyState.DeathSequenceActor->GetSequencePlayer();
	if (!SequencePlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Enemy 사망 시퀀스 재생 실패: %s에 Level Sequence Asset이 설정되지 않았습니다."), *GetNameSafe(EnemyState.DeathSequenceActor));
		return false;
	}

	// 현재 장소의 시퀀스를 정지한 뒤 사망 연출 시퀀스를 처음부터 재생합니다.
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
	EnemyState.ActiveSequenceActor = EnemyState.DeathSequenceActor;
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
