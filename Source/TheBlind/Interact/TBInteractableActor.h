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
	/** 기본 상호작용 Actor는 항상 상호작용할 수 있음을 반환합니다. */
	virtual bool CanInteract() const override;

	/** 상호작용 대상 정보를 기록하고 상호작용을 시작합니다. */
	virtual bool Interact(ATBPlayerController& PC) override;
	
	
	
// ─────────────────────────────────────────────────────────────
// Component
// ─────────────────────────────────────────────────────────────
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> InteractionBox;

};
