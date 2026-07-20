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
	UPROPERTY(EditDefaultsOnly, Category = "변수")
	TSoftObjectPtr<UWorld> StreamingLevel;

	UPROPERTY(EditInstanceOnly, Category = "변수")
	TWeakObjectPtr<AActor> StreamingCameraActor;

private:
	int32 StreamingRequestSerial = 0;
	uint8 bShouldStreamingLevelBeLoaded : 1 = false;
	uint8 bIsStreamingLevelLoaded : 1 = false;

};
