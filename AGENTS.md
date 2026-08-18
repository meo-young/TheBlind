# Unreal Engine C++ 작성 규칙

아래 규칙은 Unreal Engine C++ 파일을 작성하거나 수정할 때 적용합니다.

- 헤더의 기능별 섹션은 아래 형식을 그대로 사용합니다. 구분선의 문자, 길이, 형태를 임의로 변경하지 않습니다.

```cpp
// ─────────────────────────────────────────────────────────────
// Section Name
// ─────────────────────────────────────────────────────────────
public:
```

- 각 섹션 사이는 반드시 빈 줄 2줄로 구분합니다.
- 헤더 함수에는 기능을 설명하는 간결한 한국어 `/** ... */` 주석을 작성합니다.
- 모든 주석은 `~합니다.` 형태의 존댓말 문체로 작성합니다.
- C++ 구현부에는 모든 줄이 아닌 논리적인 코드 블록 단위로 간략한 `// ...합니다.` 주석을 작성합니다.
- 구현부 주석에는 코드만으로 명확하지 않은 의도, 처리 순서, 우선순위, 예외 및 폴백 동작을 설명합니다.
- 주석 없이도 동작과 의도를 바로 이해할 수 있는 직관적인 코드는 주석을 생략합니다.
- 람다 등 가독성을 위해 필요한 경우를 제외하면 헤더와 구현부의 함수 선언, 호출 및 코드를 불필요하게 여러 줄로 나누지 않습니다.
- 객체의 생명주기와 호출 시점상 유효성이 보장된 경우에는 습관적인 `IsValid`, `nullptr` 검사 및 조기 반환을 추가하지 않습니다.
- 게임 시작 이후의 `PlayerController`, `GameInstanceSubsystem`, 게임 내내 존재한다고 보장된 객체 등은 별도의 유효성 검사 없이 사용합니다.
- 실제로 객체가 없을 수 있거나 생명주기가 불확실한 경우에만 유효성을 검사합니다.
- `UE_LOG`의 Verbosity로 `Log`를 사용하지 않습니다.
- 일반적인 정보 및 상태 로그에는 `Warning`을 사용합니다.
- `nullptr`이 확인되거나 크래시, 데이터 손상 등 위험한 상황에는 `Error`를 사용합니다.
- 생성자 내부의 초기화 코드는 역할별 섹션으로 구분하고, 각 섹션 위에 해당 작업을 설명하는 `// ...합니다.` 주석을 작성합니다.
- 생성자에서 여러 구문으로 구성된 섹션은 독립된 중괄호 블록으로 묶습니다. 단일 구문 섹션에는 불필요한 중괄호를 추가하지 않습니다.
- `UFUNCTION`은 함수 바인딩, RPC 등 Unreal 리플렉션이 반드시 필요한 경우에만 사용합니다.
- 별도 요청이 없다면 Blueprint 사용이나 노출을 고려하지 않습니다.
- `UPROPERTY`의 `Category`는 반드시 `"변수|세부 분류"` 형식으로 작성합니다.
- 나머지는 프로젝트의 기존 스타일과 Unreal Engine 명명 규칙을 따릅니다.

## 섹션 작성 예시

```cpp
// ─────────────────────────────────────────────────────────────
// Scalability Preset
// ─────────────────────────────────────────────────────────────
public:
	/** 지정한 품질 단계에 해당하는 렌더링 설정과 해상도 스케일을 적용합니다. */
	void ApplyRenderingQualityLevel(ERenderingQualityLevel QualityLevel);

	/** 화면 해상도 스케일을 백분율 값으로 설정하고 즉시 적용합니다. */
	void SetScreenResolution(float InResolution);


// ─────────────────────────────────────────────────────────────
// Singleton
// ─────────────────────────────────────────────────────────────
public:
	/** WorldContextObject으로 RenderingQualitySubsystem을 반환합니다. */
	static URenderingQualitySubsystem& Get(const UObject* WorldContextObject);
```

## 생성자 작성 예시

```cpp
// 캐릭터 기본 속성을 설정합니다.
{
	bUseControllerRotationYaw = true;
}

// 공격 Ability가 활성화된 동안 플레이어 이동 입력을 차단합니다.
MovementBlockingTags.Add(ECGameplayTags::Status_Attacking.GetTag());

// 1인칭 카메라를 생성합니다.
{
	CameraComponent = CreateDefaultSubobject<UECCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	CameraComponent->bUsePawnControlRotation = true;
}
```

## 구현부 주석 작성 예시

```cpp
// 앞쪽 규칙이 우선합니다. 태그를 모두 충족하는 첫 Mesh를 반환합니다.
for (const FECAnimBodyStyleSelectionEntry& Rule : MeshRules)
{
	if ((Rule.Mesh) && CosmeticTags.HasAll(Rule.RequiredTags))
	{
		return Rule.Mesh;
	}
}

// 매칭 실패 시 폴백을 반환합니다.
return DefaultMesh;
```
