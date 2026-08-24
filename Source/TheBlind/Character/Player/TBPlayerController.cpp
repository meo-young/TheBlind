#include "TBPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "TBGameplayTags.h"
#include "TBPlayerState.h"
#include "AbilitySystem/TBAbilitySystemComponent.h"
#include "Actor/TBMonitor.h"
#include "Camera/TBPlayerCameraManager.h"
#include "Input/TBInputComponent.h"
#include "Interact/Interactable.h"
#include "Subsystem/RenderingQualitySubsystem.h"
#include "UserSettings/EnhancedInputUserSettings.h"

ATBPlayerController::ATBPlayerController()
{
	PlayerCameraManagerClass = ATBPlayerCameraManager::StaticClass();
}

void ATBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ATBPlayerCameraManager* CameraManager = CastChecked<ATBPlayerCameraManager>(PlayerCameraManager);
	CameraManager->OnCameraTransitionFinished().AddUObject(this, &ThisClass::HandleCameraTransitionFinished);

	// 1인칭 조작을 위해 커서를 숨기고 마우스를 게임 화면에 캡처합니다.
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ATBPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UTBInputComponent* IC = CastChecked<UTBInputComponent>(InputComponent);
	IC->BindAbilityAction(ControllerInputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased);
}

void ATBPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (const ATBPlayerState* PS = GetPlayerState<ATBPlayerState>())
	{
		if (UTBAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->ProcessAbilityInput(DeltaTime, bGamePaused);
		}
	}

	Super::PostProcessInput(DeltaTime, bGamePaused);
}

bool ATBPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	// 원격 화면에서는 실제 커서 대신 마우스 이동과 왼쪽 버튼을 모니터의 가상 커서로 전달합니다.
	if (ATBMonitor* Monitor = ActiveMonitor.Get())
	{
		if (Params.Event == IE_Axis)
		{
			if (Params.Key == EKeys::MouseX)
			{
				Monitor->MoveCCTVCursor(FVector2D(Params.AmountDepressed, 0.0f));
				return true;
			}

			if (Params.Key == EKeys::MouseY)
			{
				Monitor->MoveCCTVCursor(FVector2D(0.0f, -Params.AmountDepressed));
				return true;
			}
		}

		if (Params.Key == EKeys::LeftMouseButton)
		{
			if (Params.Event == IE_Pressed)
			{
				Monitor->PressCCTVPointer();
				return true;
			}

			if (Params.Event == IE_Released)
			{
				Monitor->ReleaseCCTVPointer();
				return true;
			}
		}
	}

	if (Params.Event == IE_Pressed)
	{
		if (Params.Key == EKeys::One)
		{
			URenderingQualitySubsystem::Get(this).ApplyRenderingQualityLevel(ERenderingQualityLevel::Low);
			return true;
		}

		if (Params.Key == EKeys::Two)
		{
			URenderingQualitySubsystem::Get(this).ApplyRenderingQualityLevel(ERenderingQualityLevel::Medium);
			return true;
		}

		if (Params.Key == EKeys::Three)
		{
			URenderingQualitySubsystem::Get(this).ApplyRenderingQualityLevel(ERenderingQualityLevel::High);
			return true;
		}
	}

	return Super::InputKey(Params);
}

IInteractable* ATBPlayerController::FindInteractionTarget() const
{
	// 현재 카메라 중앙에서 상호작용 거리까지 플레이어 Pawn을 제외하고 탐색합니다.
	const FVector TraceStart = PlayerCameraManager->GetCameraLocation();
	const FVector TraceEnd = TraceStart + PlayerCameraManager->GetCameraRotation().Vector() * InteractionRange;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PlayerInteraction), false, GetPawn());
	FHitResult HitResult;
	if (!GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return nullptr;
	}

	// 가장 먼저 감지된 Actor가 상호작용 인터페이스를 구현한 경우에만 반환합니다.
	return Cast<IInteractable>(HitResult.GetActor());
}

void ATBPlayerController::SetActiveMonitor(ATBMonitor* InMonitor)
{
	ActiveMonitor = InMonitor;
}

void ATBPlayerController::HandleCameraTransitionFinished(const ETBCameraTransitionDirection FinishedDirection)
{
	if (FinishedDirection == ETBCameraTransitionDirection::Forward)
	{
		SetRemoteViewInputEnabled(true);
		if (ATBMonitor* Monitor = ActiveMonitor.Get())
		{
			Monitor->HandleRemoteViewEntered();
		}
		return;
	}

	if (FinishedDirection == ETBCameraTransitionDirection::Reverse)
	{
		SetRemoteViewInputEnabled(false);
		if (ATBMonitor* Monitor = ActiveMonitor.Get())
		{
			Monitor->HandleRemoteViewExited();
		}
		ActiveMonitor.Reset();

		if (APawn* PlayerPawn = GetPawn())
		{
			FGameplayEventData Payload;
			Payload.EventTag = TBGameplayTags::GameplayEvent_Interact_Finished;
			Payload.Instigator = PlayerPawn;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(PlayerPawn, TBGameplayTags::GameplayEvent_Interact_Finished, Payload);
		}
	}
}

void ATBPlayerController::SetRemoteViewInputEnabled(const bool bEnabled)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	const UInputMappingContext* MappingContext = RemoteViewIMC.LoadSynchronous();
	if (!InputSubsystem || !MappingContext)
	{
		return;
	}

	if (!bEnabled)
	{
		InputSubsystem->RemoveMappingContext(MappingContext);
		return;
	}

	if (UEnhancedInputUserSettings* Settings = InputSubsystem->GetUserSettings())
	{
		Settings->RegisterInputMappingContext(MappingContext);
	}

	FModifyContextOptions Options;
	Options.bIgnoreAllPressedKeysUntilRelease = true;
	InputSubsystem->AddMappingContext(MappingContext, 1, Options);
}

void ATBPlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(TBGameplayTags::InputTag_RemoteBack))
	{
		ATBPlayerCameraManager* CameraManager = CastChecked<ATBPlayerCameraManager>(PlayerCameraManager);
		ATBMonitor* Monitor = ActiveMonitor.Get();
		if (Monitor)
		{
			Monitor->HandleRemoteViewExitStarted();
		}

		if (CameraManager->ReverseCameraTransition())
		{
			SetRemoteViewInputEnabled(false);
			return;
		}

		if (Monitor)
		{
			Monitor->HandleRemoteViewEntered();
		}
		return;
	}

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
	if (InputTag.MatchesTagExact(TBGameplayTags::InputTag_RemoteBack))
	{
		return;
	}

	if (const ATBPlayerState* PS = GetPlayerState<ATBPlayerState>())
	{
		if (UTBAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
		{
			ASC->AbilityInputTagReleased(InputTag);
		}
	}
}
