#include "TBPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "TBGameplayTags.h"
#include "TBPlayerState.h"
#include "AbilitySystem/TBAbilitySystemComponent.h"
#include "Camera/TBCameraComponent.h"
#include "Camera/TBCameraDirector.h"
#include "Camera/TBPlayerCameraManager.h"
#include "Camera/TBRemoteCameraRig.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Input/TBInputComponent.h"
#include "Interact/TBPossessableActor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

ATBPlayerController::ATBPlayerController()
{
	PlayerCameraManagerClass = ATBPlayerCameraManager::StaticClass();
}

void ATBPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UTBInputComponent* IC = CastChecked<UTBInputComponent>(InputComponent);

	IC->BindNativeAction(
		ControllerInputConfig,
		TBGameplayTags::InputTag_RemoteBack,
		ETriggerEvent::Started,
		this,
		&ThisClass::Input_RemoteBack);

	IC->BindAbilityAction(
		ControllerInputConfig,
		this,
		&ThisClass::Input_AbilityInputTagPressed,
		&ThisClass::Input_AbilityInputTagReleased);
}

void ATBPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (const ATBPlayerState* PS = GetPlayerState<ATBPlayerState>())
	{
		if (UTBAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->ProcessAbilityInput(DeltaTime,bGamePaused);
		}
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ATBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 각 단계에는 해당 상태로 진입할 때 사용할 전환 방식까지 함께 둡니다.
	CameraSteps =
	{
		{ TBGameplayTags::CameraState_Player,       FGameplayTag(),                              0.0f },
		{ TBGameplayTags::CameraState_Entry,        TBGameplayTags::CameraTransition_Blend, EntryBlendDuration },
		{ TBGameplayTags::CameraState_Remote_Idle,  TBGameplayTags::CameraTransition_Cut,        0.0f },
		{ TBGameplayTags::CameraState_Remote_Hold,  TBGameplayTags::CameraTransition_Blend, RemoteHoldBlendDuration },
		{ TBGameplayTags::CameraState_Remote_Final, TBGameplayTags::CameraTransition_Cut,        0.0f }
	};

	CompleteCameraStep(0);
	TargetCameraStep = CurrentCameraStep;

	bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputMode);
}

void ATBPlayerController::SetRemoteViewInputEnabled(const bool bEnabled)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (const UInputMappingContext* IMC = RemoteViewIMC.LoadSynchronous())
		{
			if (bEnabled)
			{
				Subsystem->AddMappingContext(IMC, 100);
			}
			else
			{
				Subsystem->RemoveMappingContext(IMC);
			}
		}
	}
}

void ATBPlayerController::Input_RemoteBack(const FInputActionValue& InputActionValue)
{
	if (ActivePossessableActor.IsValid())
	{
		RequestCameraState(TBGameplayTags::CameraState_Player);
	}
}

void ATBPlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const ATBPlayerState* PS = GetPlayerState<ATBPlayerState>())
	{
		if (UTBAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->AbilityInputTagPressed(InputTag);
		}
	}
}

void ATBPlayerController::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (const ATBPlayerState* PS = GetPlayerState<ATBPlayerState>())
	{
		if (UTBAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void ATBPlayerController::BeginRemoteSequence(ATBPossessableActor& SourceActor)
{
	// 시퀀스가 이미 진행 중이라면 중복 실행하지 않습니다.
	if (ActivePossessableActor.IsValid())
	{
		return;
	}

	if (SourceActor.StreamingLevel.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("상호작용 오브젝트에 StreamingLevel이 지정되지 않았습니다."));
		return;
	}

	const FString LevelPackageName = FPackageName::ObjectPathToPackageName(
		SourceActor.StreamingLevel.ToString());

	ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(
		this,
		FName(*LevelPackageName));

	if (!IsValid(StreamingLevel))
	{
		UE_LOG(LogTemp, Error, TEXT("LV_Main에 등록된 스트리밍 레벨을 찾지 못했습니다: %s"),
			*LevelPackageName);
		return;
	}

	// CameraDirector는 최초 상호작용 시 한 번만 생성하고 재사용합니다.
	if (!IsValid(CameraDirector))
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CameraDirector = GetWorld()->SpawnActor<ATBCameraDirector>(
				ATBCameraDirector::StaticClass(),
				FTransform::Identity,
				SpawnParameters);
	}

	if (!IsValid(CameraDirector))
	{
		UE_LOG(LogTemp, Error, TEXT("CameraDirector를 생성하지 못했습니다."));
		return;
	}

	CameraDirector->OnBlendFinished.RemoveAll(this);
	CameraDirector->OnBlendFinished.AddUObject(this, &ThisClass::HandleCameraBlendFinished);

	ActivePossessableActor = &SourceActor;
	ActiveStreamingLevel = StreamingLevel;
	ActiveRemoteCameraRig.Reset();

	StreamingLevel->OnLevelShown.RemoveDynamic(this, &ThisClass::HandleRemoteLevelShown);
	StreamingLevel->OnLevelShown.AddUniqueDynamic(this, &ThisClass::HandleRemoteLevelShown);

	// ViewTarget을 바꾸기 직전의 플레이어 화면을 보존합니다.
	const FMinimalViewInfo InitialView = PlayerCameraManager->GetCameraCacheView();

	// 현재 플레이어 화면과 동일하게 Director의 시점을 설정합니다.
	CameraDirector->SetViewImmediately(InitialView);

	// 동일한 화면 상태에서 ViewTarget만 교체하므로 화면이 튀지 않습니다.
	SetViewTarget(CameraDirector);

	SetRemoteViewInputEnabled(true);

	if (ATBPlayerState* PS = GetPlayerState<ATBPlayerState>())
	{
		if (UTBAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->SetLooseGameplayTagCount(TBGameplayTags::Status_Remote, 1);
		}
	}

	RequestCameraState(TBGameplayTags::CameraState_Remote_Idle);

	// EntryCamera로 이동하는 동안 스트리밍 레벨을 비동기로 준비합니다.
	StreamingLevel->SetShouldBeLoaded(true);
	StreamingLevel->SetShouldBeVisible(true);

	// 이미 표시된 레벨은 OnLevelShown이 다시 호출되지 않을 수 있습니다.
	if (StreamingLevel->IsLevelVisible())
	{
		HandleRemoteLevelShown();
	}
}

void ATBPlayerController::FinishRemoteSequence()
{
	// CameraDirector와 플레이어 카메라의 시점이 같은 상태에서
	// ViewTarget을 다시 Pawn으로 돌립니다.
	SetViewTarget(GetPawn());

	SetRemoteViewInputEnabled(false);

	// 플레이어 시점까지 돌아온 뒤 Remote 카메라 레벨을 해제합니다.
	if (ActiveStreamingLevel.IsValid())
	{
		ActiveStreamingLevel->OnLevelShown.RemoveDynamic(
			this,
			&ThisClass::HandleRemoteLevelShown);
		ActiveStreamingLevel->SetShouldBeVisible(false);
		ActiveStreamingLevel->SetShouldBeLoaded(false);
	}

	ActiveRemoteCameraRig.Reset();
	ActiveStreamingLevel.Reset();

	if (ATBPlayerState* PS = GetPlayerState<ATBPlayerState>())
	{
		if (UTBAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->SetLooseGameplayTagCount(TBGameplayTags::Status_Remote, 0);
		}
	}

	// GA_Interact가 기다리고 있는 완료 이벤트를 전송합니다.
	FGameplayEventData Payload;
	Payload.EventTag = TBGameplayTags::GameplayEvent_Interact_Finished;
	Payload.Instigator = GetPawn();
	Payload.Target = ActivePossessableActor.Get();

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetPawn(), TBGameplayTags::GameplayEvent_Interact_Finished, Payload);

	ActivePossessableActor.Reset();
}

bool ATBPlayerController::BeginRemoteHold()
{
	if (!ActivePossessableActor.IsValid() ||
		!IsCurrentCameraState(TBGameplayTags::CameraState_Remote_Idle))
	{
		return false;
	}

	RequestCameraState(TBGameplayTags::CameraState_Remote_Final);
	return true;
}

void ATBPlayerController::EndRemoteHold()
{
	if (!ActivePossessableActor.IsValid())
	{
		return;
	}

	const int32 PlayerStep = FindCameraStepIndex(TBGameplayTags::CameraState_Player);
	if (TargetCameraStep != PlayerStep)
	{
		RequestCameraState(TBGameplayTags::CameraState_Remote_Idle);
	}
}

void ATBPlayerController::RequestCameraState(const FGameplayTag& NewDesiredState)
{
	const int32 NewTargetStep = FindCameraStepIndex(NewDesiredState);
	if (!CameraSteps.IsValidIndex(NewTargetStep))
	{
		UE_LOG(LogTemp, Error, TEXT("등록되지 않은 카메라 상태입니다: %s"), *NewDesiredState.ToString());
		return;
	}

	TargetCameraStep = NewTargetStep;
	if (PendingCameraStep == INDEX_NONE)
	{
		AdvanceCameraState();
	}
}

bool ATBPlayerController::IsCurrentCameraState(const FGameplayTag& CameraState) const
{
	return PendingCameraStep == INDEX_NONE &&
		CameraSteps.IsValidIndex(CurrentCameraStep) &&
		CameraSteps[CurrentCameraStep].StateTag.MatchesTagExact(CameraState);
}

void ATBPlayerController::AdvanceCameraState()
{
	if (!IsValid(CameraDirector) || PendingCameraStep != INDEX_NONE)
	{
		return;
	}

	while (CurrentCameraStep != TargetCameraStep)
	{
		const int32 Direction = TargetCameraStep > CurrentCameraStep ? 1 : -1;
		const int32 NextStep = CurrentCameraStep + Direction;
		const int32 TransitionStep = FMath::Max(CurrentCameraStep, NextStep);

		if (!CameraSteps.IsValidIndex(NextStep) || !CameraSteps.IsValidIndex(TransitionStep))
		{
			return;
		}

		const FTBCameraStep& Next = CameraSteps[NextStep];
		const FTBCameraStep& Transition = CameraSteps[TransitionStep];

		FMinimalViewInfo TargetView;
		if (!ResolveCameraView(Next.StateTag, TargetView))
		{
			// 스트리밍 카메라가 준비되면 같은 목표 상태를 다시 요청합니다.
			return;
		}

		if (Transition.TransitionTag.MatchesTagExact(TBGameplayTags::CameraTransition_Cut))
		{
			CameraDirector->SetViewImmediately(TargetView);
			if (PlayerCameraManager)
			{
				PlayerCameraManager->SetGameCameraCutThisFrame();
			}

			CompleteCameraStep(NextStep);
			continue;
		}

		if (Transition.TransitionTag.MatchesTagExact(TBGameplayTags::CameraTransition_Blend))
		{
			PendingCameraStep = NextStep;
			CameraDirector->BlendTo(TargetView, Transition.Duration);
			return;
		}

		return;
	}

	if (ActivePossessableActor.IsValid() && IsCurrentCameraState(TBGameplayTags::CameraState_Player))
	{
		FinishRemoteSequence();
	}
}

void ATBPlayerController::HandleCameraBlendFinished()
{
	if (!CameraSteps.IsValidIndex(PendingCameraStep))
	{
		return;
	}

	const int32 CompletedStep = PendingCameraStep;
	PendingCameraStep = INDEX_NONE;
	CompleteCameraStep(CompletedStep);
	AdvanceCameraState();
}

void ATBPlayerController::CompleteCameraStep(const int32 StepIndex)
{
	if (!CameraSteps.IsValidIndex(StepIndex) || CurrentCameraStep == StepIndex)
	{
		return;
	}

	if (ATBPlayerState* PS = GetPlayerState<ATBPlayerState>())
	{
		if (UTBAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			if (CameraSteps.IsValidIndex(CurrentCameraStep))
			{
				ASC->SetLooseGameplayTagCount(CameraSteps[CurrentCameraStep].StateTag, 0);
			}

			ASC->SetLooseGameplayTagCount(CameraSteps[StepIndex].StateTag, 1);
		}
	}

	CurrentCameraStep = StepIndex;

	UE_LOG(LogTemp, Warning, TEXT("카메라 상태 변경: %s"),
		*CameraSteps[CurrentCameraStep].StateTag.ToString());
}

int32 ATBPlayerController::FindCameraStepIndex(const FGameplayTag& CameraState) const
{
	return CameraSteps.IndexOfByPredicate(
		[&CameraState](const FTBCameraStep& Step)
		{
			return Step.StateTag.MatchesTagExact(CameraState);
		});
}

bool ATBPlayerController::ResolveCameraView(const FGameplayTag& CameraState, FMinimalViewInfo& OutView) const
{
	if (CameraState.MatchesTagExact(TBGameplayTags::CameraState_Player))
	{
		if (APawn* ControlledPawn = GetPawn())
		{
			ControlledPawn->CalcCamera(0.0f, OutView);
			return true;
		}

		return false;
	}

	if (CameraState.MatchesTagExact(TBGameplayTags::CameraState_Entry))
	{
		if (ActivePossessableActor.IsValid() && IsValid(ActivePossessableActor->EntryCamera))
		{
			ActivePossessableActor->EntryCamera->GetCameraView(0.0f, OutView);
			return true;
		}

		return false;
	}

	if (!ActiveRemoteCameraRig.IsValid())
	{
		return false;
	}

	UTBCameraComponent* TargetCamera = nullptr;

	if (CameraState.MatchesTagExact(TBGameplayTags::CameraState_Remote_Idle))
	{
		TargetCamera = ActiveRemoteCameraRig->RemoteIdleCamera;
	}
	else if (CameraState.MatchesTagExact(TBGameplayTags::CameraState_Remote_Hold))
	{
		TargetCamera = ActiveRemoteCameraRig->RemoteHoldCamera;
	}
	else if (CameraState.MatchesTagExact(TBGameplayTags::CameraState_Remote_Final))
	{
		TargetCamera = ActiveRemoteCameraRig->RemoteFinalCamera;
	}

	if (!IsValid(TargetCamera))
	{
		return false;
	}

	TargetCamera->GetCameraView(0.0f, OutView);
	return true;
}

void ATBPlayerController::HandleRemoteLevelShown()
{
	ActiveRemoteCameraRig.Reset();

	if (!ActiveStreamingLevel.IsValid())
	{
		return;
	}

	ULevel* LoadedLevel = ActiveStreamingLevel->GetLoadedLevel();
	if (!IsValid(LoadedLevel))
	{
		return;
	}

	ATBRemoteCameraRig* FoundRig = nullptr;

	for (AActor* Actor : LoadedLevel->Actors)
	{
		ATBRemoteCameraRig* Rig = Cast<ATBRemoteCameraRig>(Actor);
		if (!IsValid(Rig))
		{
			continue;
		}

		// 한 레벨에 리그가 두 개 이상 있으면 잘못된 레벨 구성입니다.
		if (IsValid(FoundRig))
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("스트리밍 레벨에는 RemoteCameraRig가 하나만 있어야 합니다."));
			return;
		}

		FoundRig = Rig;
	}

	if (!IsValid(FoundRig))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("스트리밍 레벨에서 RemoteCameraRig를 찾지 못했습니다."));
		return;
	}

	ActiveRemoteCameraRig = FoundRig;

	// RemoteIdle을 향해 진행하다 멈춘 상태를 재개합니다.
	AdvanceCameraState();
}
