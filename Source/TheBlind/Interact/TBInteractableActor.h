#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "TBInteractableActor.generated.h"

class UBoxComponent;
class UPaperFlipbookComponent;

UCLASS()
class THEBLIND_API ATBInteractableActor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// Actor Interface
// ─────────────────────────────────────────────────────────────
public:
	ATBInteractableActor();
	
	

// ─────────────────────────────────────────────────────────────
// Interactable Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void Interact() override;
	
	
	
// ─────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPaperFlipbookComponent> FlipbookComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> InteractionBox;

};
