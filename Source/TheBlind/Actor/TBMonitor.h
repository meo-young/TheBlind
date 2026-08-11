#pragma once

#include "CoreMinimal.h"
#include "Interact/TBPossessableActor.h"
#include "TBMonitor.generated.h"

UCLASS()
class THEBLIND_API ATBMonitor : public ATBPossessableActor
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────	
// Actor Interface
// ─────────────────────────────────────────────────────────────	
public:
	ATBMonitor();
	
	
// ─────────────────────────────────────────────────────────────	
// Components
// ─────────────────────────────────────────────────────────────
protected:
	/** 집 안의 여러 장소를 렌더링하는 모니터 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Monitor;
	
};
