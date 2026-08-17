#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "TBPlayerController.generated.h"

class ATBMonitor;
class UTBInputConfig;
class UInputMappingContext;
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
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PostProcessInput(float DeltaTime, bool bGamePaused) override;
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;


// ─────────────────────────────────────────────────────────────
// Remote View
// ─────────────────────────────────────────────────────────────
public:
	/** 현재 원격 화면을 표시하는 모니터를 설정합니다. */
	void SetActiveMonitor(ATBMonitor* InMonitor);

private:
	/** 카메라 전환 완료 방향에 따라 모니터의 원격 시점 상태를 갱신합니다. */
	void HandleCameraTransitionFinished(ETBCameraTransitionDirection FinishedDirection);

	/** 원격 화면 전용 입력 매핑 컨텍스트의 활성화 여부를 설정합니다. */
	void SetRemoteViewInputEnabled(bool bEnabled);


// ─────────────────────────────────────────────────────────────
// Ability Input
// ─────────────────────────────────────────────────────────────
private:
	/** InputTag 입력에 따라 Ability를 발동시키기 위한 바인딩 함수입니다. */
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);

	/** InputTag 입력 종료에 따라 Ability를 중단하기 위한 바인딩 함수입니다. */
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);


// ─────────────────────────────────────────────────────────────
// Input Configuration
// ─────────────────────────────────────────────────────────────
protected:
	/** Controller의 Ability 입력 바인딩 정보를 보관합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TObjectPtr<UTBInputConfig> ControllerInputConfig;

	/** 모니터 원격 화면에서 사용하는 입력 매핑 컨텍스트입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|입력")
	TSoftObjectPtr<UInputMappingContext> RemoteViewIMC;


// ─────────────────────────────────────────────────────────────
// Runtime State
// ─────────────────────────────────────────────────────────────
private:
	/** 현재 원격 화면을 표시하는 모니터입니다. */
	TWeakObjectPtr<ATBMonitor> ActiveMonitor;
};
