#include "TBPlayerController.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "TBGameplayTags.h"
#include "TBPlayerState.h"
#include "AbilitySystem/TBAbilitySystemComponent.h"
#include "Camera/TBPlayerCameraManager.h"
#include "Input/TBInputComponent.h"
#include "Interact/Interactable.h"

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
		TBGameplayTags::InputTag_Interact,
		ETriggerEvent::Started,
		this,
		&ThisClass::Input_Interact);

	IC->BindNativeAction(
		ControllerInputConfig,
		TBGameplayTags::InputTag_RemoteAction,
		ETriggerEvent::Started,
		this,
		&ThisClass::Input_RemoteAction_Started);
			
	IC->BindNativeAction(
		ControllerInputConfig,
		TBGameplayTags::InputTag_RemoteAction,
		ETriggerEvent::Completed,
		this,
		&ThisClass::Input_RemoteAction_Released);
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
	
	bShowMouseCursor = true;
	
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetInputMode(InputMode);
}

void ATBPlayerController::Input_Interact(const FInputActionValue& InputActionValue)
{
	FHitResult HitResult;

	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	if (!GetHitResultUnderCursorByChannel(TraceChannel, false, HitResult))
	{
		return;
	}

	if (AActor* TargetActor = HitResult.GetActor())
	{
		if (IInteractable* Interactable = Cast<IInteractable>(TargetActor))
		{
			const float DistanceSquared = FVector::Dist2D(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
			if (DistanceSquared <= InteractionRange)
			{
				Interactable->Interact();
			}
		}
	}
}

void ATBPlayerController::Input_RemoteAction_Started(const FInputActionValue& InputActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Remote 시작"))
}

void ATBPlayerController::Input_RemoteAction_Released(const FInputActionValue& InputActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Remote 끝"))
}

void ATBPlayerController::AddRemoteViewIMC()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (const UInputMappingContext* IMC = RemoteViewIMC.LoadSynchronous())
		{
			Subsystem->AddMappingContext(IMC, 100);
		}
	}
}

void ATBPlayerController::RemoveRemoteViewIMC()
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (const UInputMappingContext* IMC = RemoteViewIMC.LoadSynchronous())
		{
			Subsystem->RemoveMappingContext(IMC);
		}
	}
}
