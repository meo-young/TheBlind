#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "TBCCTVWidgetComponent.generated.h"

UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class THEBLIND_API UTBCCTVWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

// ─────────────────────────────────────────────────────────────
// ActorComponent Interface
// ─────────────────────────────────────────────────────────────
public:
	UTBCCTVWidgetComponent();

protected:
	virtual void OnRegister() override;
};
