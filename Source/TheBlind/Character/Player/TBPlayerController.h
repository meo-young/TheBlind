#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "TBPlayerController.generated.h"

class ATBPossessableActor;
class UTBInputConfig;
class UInputMappingContext;
struct FInputActionValue;
enum class ETBCameraTransitionDirection : int8;

UCLASS()
class THEBLIND_API ATBPlayerController : public APlayerController
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// PlayerController Interface
// ─────────────────────────────────────────────────────────────
public:
	ATBPlayerController();
	virtual void SetupInputComponent() override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void BeginPlay() override;

	
	
// ─────────────────────────────────────────────────────────────
// Input
// ─────────────────────────────────────────────────────────────	
protected:	
	/** InputTag 입력에 따라 Ability를 발동시키기 위한 바인딩 함수입니다. */
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);

	/** InputTag 입력 종료에 따라 Ability를 중단하기 위한 바인딩 함수입니다. */
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	void HandleCameraTransitionFinished(ETBCameraTransitionDirection FinishedDirection);
	void SetRemoteViewInputEnabled(bool bEnabled);
	
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TObjectPtr<UTBInputConfig> ControllerInputConfig;

	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TSoftObjectPtr<UInputMappingContext> RemoteViewIMC;
};
