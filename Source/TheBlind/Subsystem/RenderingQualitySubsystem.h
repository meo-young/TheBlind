#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RenderingQualitySubsystem.generated.h"

UENUM()
enum class ERenderingQualityLevel : uint8
{
	High,
	Medium,
	Low
};

UCLASS()
class THEBLIND_API URenderingQualitySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────	
// GameInstanceSubsystem Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	
// ─────────────────────────────────────────────────────────────	
// Scalability Preset
// ─────────────────────────────────────────────────────────────
public:
	/** 지정한 품질 단계에 해당하는 렌더링 설정과 해상도 스케일을 적용합니다. */
	void ApplyRenderingQualityLevel(ERenderingQualityLevel QualityLevel);

	/** 지정한 렌더링 콘솔 변수에 게임 설정 우선순위로 값을 적용합니다. */
	void SetRenderingCVar(const TCHAR* Name, float Value);

	/** 화면 해상도 스케일을 백분율 값으로 설정하고 즉시 적용합니다. */
	void SetScreenResolution(const float InResolution);
	
	
// ─────────────────────────────────────────────────────────────	
// Singleton	
// ─────────────────────────────────────────────────────────────	
public:
	/** WorldContextObject으로 RenderingQualitySubsystem 반환합니다. */
	static URenderingQualitySubsystem& Get(const UObject* WorldContextObject);
	
};
