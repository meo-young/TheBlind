#pragma once

#include "CoreMinimal.h"
#include "TBInteractableActor.h"
#include "TBPossessableActor.generated.h"

class UTBCameraComponent;

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
	virtual void Interact(ATBPlayerController& PC) override;
	
	
	
// ─────────────────────────────────────────────────────────────
// Variable
// ─────────────────────────────────────────────────────────────
public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTBCameraComponent> EntryCamera;
	
	UPROPERTY(EditAnywhere, Category = "변수")
	TSoftObjectPtr<UWorld> StreamingLevel;

};
