#include "TBPlayerController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "TBGameplayTags.h"
#include "TBPlayerState.h"
#include "AbilitySystem/TBAbilitySystemComponent.h"
#include "Camera/TBPlayerCameraManager.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Input/TBInputComponent.h"
#include "Misc/PackageName.h"
#include "UserSettings/EnhancedInputUserSettings.h"

ATBPlayerController::ATBPlayerController()
{
	PlayerCameraManagerClass = ATBPlayerCameraManager::StaticClass();
}

void ATBPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UTBInputComponent* IC = CastChecked<UTBInputComponent>(InputComponent);

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

	if (ATBPlayerCameraManager* CameraManager = Cast<ATBPlayerCameraManager>(PlayerCameraManager))
	{
		CameraManager->OnCameraTransitionFinished().AddUObject(
			this,
			&ThisClass::HandleCameraTransitionFinished);
	}

	bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputMode);
}


void ATBPlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTagExact(TBGameplayTags::InputTag_RemoteBack))
	{
		if (ATBPlayerCameraManager* CameraManager = Cast<ATBPlayerCameraManager>(PlayerCameraManager))
		{
			if (CameraManager->ReverseCameraTransition())
			{
				SetRemoteViewInputEnabled(false);
			}
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

void ATBPlayerController::HandleCameraTransitionFinished(const ETBCameraTransitionDirection FinishedDirection)
{
	if (FinishedDirection == ETBCameraTransitionDirection::Forward)
	{
		SetRemoteViewInputEnabled(true);
		return;
	}

	if (FinishedDirection == ETBCameraTransitionDirection::Reverse)
	{
		SetRemoteViewInputEnabled(false);

		if (APawn* PlayerPawn = GetPawn())
		{
			FGameplayEventData Payload;
			Payload.EventTag = TBGameplayTags::GameplayEvent_Interact_Finished;
			Payload.Instigator = PlayerPawn;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				PlayerPawn,
				TBGameplayTags::GameplayEvent_Interact_Finished,
				Payload);
		}
	}
}

void ATBPlayerController::SetRemoteViewInputEnabled(const bool bEnabled)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!IsValid(LocalPlayer))
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	const UInputMappingContext* MappingContext = RemoteViewIMC.LoadSynchronous();
	if (!IsValid(InputSubsystem) || !IsValid(MappingContext))
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
