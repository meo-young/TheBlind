#pragma once

#include "CoreMinimal.h"
#include "TBLocationTypes.generated.h"

/** CCTV와 Enemy 이동 시스템이 공통으로 사용하는 장소 식별자입니다. */
UENUM()
enum class ETBLocation : uint8
{
	Dining UMETA(DisplayName = "식당"),
	Laundry UMETA(DisplayName = "세탁실"),
	Living UMETA(DisplayName = "거실"),
	Toilet UMETA(DisplayName = "화장실"),
	Closet UMETA(DisplayName = "옷장"),
	Basement UMETA(DisplayName = "지하실"),
	PlayerRoomApproach1 UMETA(DisplayName = "플레이어 방 직전 방1"),
	PlayerRoomApproach2 UMETA(DisplayName = "플레이어 방 직전 방2"),
	Hidden UMETA(DisplayName = "숨김")
};

/** Enemy가 이동할 때 사용하는 장소 하나의 설정입니다. */
USTRUCT()
struct FTBLocationDefinition
{
	GENERATED_BODY()

	/** 현재 장소에서 직접 이동할 수 있는 장소들입니다. */
	UPROPERTY(EditAnywhere, Category = "변수|장소")
	TArray<ETBLocation> ReachableLocations;
};
