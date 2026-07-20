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
	virtual bool Interact(ATBPlayerController& PC) = 0;
	
};
