#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class ATBPlayerController;

UINTERFACE()
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class THEBLIND_API IInteractable
{
	GENERATED_BODY()

public:
	/** 현재 플레이어가 오브젝트와 상호작용할 수 있는지 반환합니다. */
	virtual bool CanInteract() const = 0;

	/** 플레이어와 오브젝트의 상호작용을 시작합니다. */
	virtual bool Interact(ATBPlayerController& PC) = 0;
	
};
