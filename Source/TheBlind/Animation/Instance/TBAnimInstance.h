#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "PaperZDAnimInstance.h"
#include "TBAnimInstance.generated.h"

UCLASS()
class THEBLIND_API UTBAnimInstance : public UPaperZDAnimInstance
{
	GENERATED_BODY()
	
// ─────────────────────────────────────────────────────────────
// AnimInstance Interface
// ─────────────────────────────────────────────────────────────
public:
	virtual void OnInit_Implementation() override;
	
	
// ─────────────────────────────────────────────────────────────
// Initializer
// ─────────────────────────────────────────────────────────────
public:
	void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

	
// ─────────────────────────────────────────────────────────────
// Tag
// ─────────────────────────────────────────────────────────────
protected:
	/** Tag ↔ 프로퍼티 바인딩 맵 — Tag 상태 변화가 BlueprintReadOnly 변수에 자동 반영됩니다. */
	UPROPERTY(EditDefaultsOnly, Category = "변수|태그")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
	
	
};
