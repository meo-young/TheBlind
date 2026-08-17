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

	bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
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
