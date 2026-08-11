#pragma once

#include "CoreMinimal.h"
#include "TBInteractableActor.h"
#include "TBPossessableActor.generated.h"

class USphereComponent;
class UTBCameraComponent;
class ATBPlayerCameraManager;

UCLASS()
class THEBLIND_API ATBPossessableActor : public ATBInteractableActor
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────
public:
	ATBPossessableActor();
	
	
// ─────────────────────────────────────────────────────────────
// Interactable Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual bool Interact(ATBPlayerController& PC) override;
	
	
	
// ─────────────────────────────────────────────────────────────
// Callback
// ─────────────────────────────────────────────────────────────
public:
	UFUNCTION()
	void OnLevelStreamingTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnLevelStreamingTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void HandleStreamingLevelLoaded();
	
	
	
// ─────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────
public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTBCameraComponent> EntryCamera;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> LevelStreamingTrigger;

	

// ─────────────────────────────────────────────────────────────
// Streaming
// ─────────────────────────────────────────────────────────────
public:
	/**
	 * true면 EntryCamera 보간 후 StreamingCameraActor로 한 번 더 전환합니다.
	 * false면 EntryCamera에 도착한 상태로 머물며, 원격 카메라와 레벨 스트리밍을 사용하지 않습니다.
	 */
	UPROPERTY(EditAnywhere, Category = "변수|카메라")
	bool bUseRemoteCamera = true;

	UPROPERTY(EditDefaultsOnly, Category = "변수|스트리밍", meta = (EditCondition = "bUseRemoteCamera"))
	TSoftObjectPtr<UWorld> StreamingLevel;

	UPROPERTY(EditInstanceOnly, Category = "변수|카메라", meta = (EditCondition = "bUseRemoteCamera"))
	TWeakObjectPtr<AActor> StreamingCameraActor;

private:
	int32 StreamingRequestSerial = 0;
	uint8 bShouldStreamingLevelBeLoaded : 1 = false;
	uint8 bIsStreamingLevelLoaded : 1 = false;

};
