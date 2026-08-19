#include "TBEnemyDirector.h"
#include "Actor/TBMonitor.h"
#include "Core/TBGameMode.h"
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

		PlayEnemyLocationSequence(EnemyState, EnemyState.StartLocation);
		UpdatePlayerDeathTimer(EnemyState);
		ScheduleEnemyMove(EnemyIndex);
	}
}

bool ATBEnemyDirector::TryMoveEnemy(AActor& Enemy, const ETBLocation Destination)
{
	FTBEnemyLocationState* EnemyState = FindEnemyState(Enemy);
	if (!EnemyState)
	{
		return false;
	}

	// 숨김 장소로 이동한 Enemy는 재등장하기 전까지 일반 장소 이동을 처리하지 않습니다.
	if (EnemyState->CurrentLocation == ETBLocation::Hidden)
	{
		return false;
	}

	// 선택된 장소의 시퀀스를 재생한 뒤 현재 장소를 변경합니다.
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

bool ATBEnemyDirector::TryBeginEnemyRelocation(const ETBLocation SourceLocation)
{
	// 지정 장소에 있는 첫 Enemy를 찾습니다.
	for (int32 EnemyIndex = 0; EnemyIndex < EnemyStates.Num(); ++EnemyIndex)
	{
		FTBEnemyLocationState& EnemyState = EnemyStates[EnemyIndex];

		if (EnemyState.CurrentLocation != SourceLocation)
		{
			continue;
		}

		// 일반 이동과 사망 카운트다운을 중단합니다.
		GetWorldTimerManager().ClearTimer(EnemyState.MoveTimerHandle);
		GetWorldTimerManager().ClearTimer(EnemyState.PlayerDeathTimerHandle);

		// 장소 상태를 숨김으로 변경하고 Enemy를 전용 숨김 좌표로 이동합니다.
		EnemyState.CurrentLocation = ETBLocation::Hidden;
		EnemyState.Enemy->SetActorLocation(FVector(1000.0f, 1000.0f, 1000.0f));

		// 설정한 대기 시간이 지난 뒤 후보 장소 중 하나에서 Enemy를 다시 활성화합니다.
		FTimerDelegate RelocationTimerDelegate;
		RelocationTimerDelegate.BindUObject(this, &ThisClass::HandleEnemyRelocationTimer, EnemyIndex);
		GetWorldTimerManager().SetTimer(EnemyState.RelocationTimerHandle, RelocationTimerDelegate, EnemyHiddenDuration, false);

		// 시청 중인 장소에서 Enemy가 사라질 경우 CCTV 이동 노이즈를 표시합니다.
		if (CCTVMonitor)
		{
			CCTVMonitor->ShowEnemyMovementNoise(SourceLocation);
		}

		return true;
	}

	// 지정 장소에 Enemy가 없다면 실패를 반환합니다.
	return false;
}

void ATBEnemyDirector::HandleEnemyRelocationTimer(const int32 EnemyIndex)
{
	FTBEnemyLocationState& EnemyState = EnemyStates[EnemyIndex];
	if (EnemyState.CurrentLocation != ETBLocation::Hidden)
	{
		return;
	}

	// 공통 후보 중 하나를 무작위로 선택하여 해당 장소의 시퀀스를 재생합니다.
	const int32 DestinationIndex = FMath::RandRange(0, EnemyRespawnLocations.Num() - 1);
	const ETBLocation Destination = EnemyRespawnLocations[DestinationIndex];
	PlayEnemyLocationSequence(EnemyState, Destination);
	EnemyState.CurrentLocation = Destination;

	UpdatePlayerDeathTimer(EnemyState);
	ScheduleEnemyMove(EnemyIndex);

	if (CCTVMonitor)
	{
		CCTVMonitor->ShowEnemyMovementNoise(EnemyState.CurrentLocation);
	}
}

void ATBEnemyDirector::ScheduleEnemyMove(const int32 EnemyIndex)
{
	FTBEnemyLocationState& EnemyState = EnemyStates[EnemyIndex];
	if (EnemyState.CurrentLocation == ETBLocation::Hidden)
	{
		return;
	}

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
	if (EnemyState.CurrentLocation == ETBLocation::Hidden)
	{
		return;
	}

	// 현재 장소와 직접 연결된 장소 중 하나를 무작위로 선택합니다.
	const TArray<ETBLocation>& ReachableLocations = LocationGraph.Find(EnemyState.CurrentLocation)->ReachableLocations;
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
	// 예약된 이동과 사망 처리를 취소하고 현재 재생 중인 장소 시퀀스를 모두 중지합니다.
	for (FTBEnemyLocationState& EnemyState : EnemyStates)
	{
		GetWorldTimerManager().ClearTimer(EnemyState.MoveTimerHandle);
		GetWorldTimerManager().ClearTimer(EnemyState.PlayerDeathTimerHandle);
		GetWorldTimerManager().ClearTimer(EnemyState.RelocationTimerHandle);

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
	// 숨김 장소에 있는 Enemy는 플레이어 사망 카운트다운을 진행하지 않습니다.
	if (EnemyState.CurrentLocation == ETBLocation::Hidden)
	{
		GetWorldTimerManager().ClearTimer(EnemyState.PlayerDeathTimerHandle);
		return;
	}

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

	const int32 SequenceIndex = FMath::RandRange(0, SequenceSet->SequenceActors.Num() - 1);
	ALevelSequenceActor* SequenceActor = SequenceSet->SequenceActors[SequenceIndex];
	ULevelSequencePlayer* SequencePlayer = SequenceActor->GetSequencePlayer();

	// 새 장소의 시퀀스를 재생합니다.
	SequencePlayer->Play();
	EnemyState.ActiveSequenceActor = SequenceActor;
	return true;
}

bool ATBEnemyDirector::PlayEnemyDeathSequence(FTBEnemyLocationState& EnemyState)
{
	ULevelSequencePlayer* SequencePlayer = EnemyState.DeathSequenceActor->GetSequencePlayer();

	// 사망 연출 시퀀스를 재생합니다.
	SequencePlayer->Play();
	EnemyState.ActiveSequenceActor = EnemyState.DeathSequenceActor;
	return true;
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
