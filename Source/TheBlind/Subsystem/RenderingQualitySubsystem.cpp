#include "RenderingQualitySubsystem.h"
#include "GameFramework/GameUserSettings.h"

void URenderingQualitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	ApplyRenderingQualityLevel(ERenderingQualityLevel::High);
}

void URenderingQualitySubsystem::ApplyRenderingQualityLevel(ERenderingQualityLevel QualityLevel)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!IsValid(Settings))
	{
		return;
	}
	
	SetRenderingCVar(TEXT("r.LumenScene.Radiosity.Temporal.MaxFramesAccumulated"), 1024);
	SetRenderingCVar(TEXT("t.MaxFPS"), 120.0f);
	
	Settings->SetViewDistanceQuality(0);
	Settings->SetGlobalIlluminationQuality(2);
	Settings->SetTextureQuality(0);
	Settings->SetFoliageQuality(0);
	Settings->SetVisualEffectQuality(0);
	Settings->SetShadingQuality(0);
	Settings->SetShadowQuality(3);
	Settings->SetAntiAliasingQuality(0);
	Settings->SetPostProcessingQuality(0);
	Settings->SetReflectionQuality(0);
	
	switch (QualityLevel)
	{
		case ERenderingQualityLevel::Low:
			SetScreenResolution(50.0f);
		break;
	
		case ERenderingQualityLevel::Medium:
			SetScreenResolution(75.0f);
		break;
		
		case ERenderingQualityLevel::High:
			SetScreenResolution(100.0f);
		break;
		
	default:
		return;
		
	}
	
	Settings->ApplySettings(false);
}

void URenderingQualitySubsystem::SetRenderingCVar(const TCHAR* Name, float Value)
{
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(Name))
	{
		CVar->Set(Value, ECVF_SetByGameSetting);
	}
}

void URenderingQualitySubsystem::SetScreenResolution(const float InResolution)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!IsValid(Settings))
	{
		return;
	}
	
	Settings->SetResolutionScaleValueEx(InResolution);
	Settings->ApplySettings(false);
}

URenderingQualitySubsystem& URenderingQualitySubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	URenderingQualitySubsystem* Router = UGameInstance::GetSubsystem<URenderingQualitySubsystem>(World->GetGameInstance());
	check(Router);
	return *Router;
}