// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTags.h"
#include "KFBBUtility.generated.h"


class UKFBB_FieldTile;

/**
 * 
 */
UCLASS(meta = (BlueprintThreadSafe, ScriptName = "KFBB Utility"))
class KFBB_API UKFBBUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};

UCLASS(Abstract)
class KFBB_API UTagLibrary : public UObject
{
	GENERATED_BODY()

public:
	static FGameplayTag StatusPlayerHasBall;

//	FGameplayTag ProhibitCommandsTag;
//	FGameplayTag TriggerPickupAbilityTag;
	
};

USTRUCT(BlueprintType)
struct FTileInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 TileIdx;

	FTileInfo();
	FTileInfo(int32 idx);
	FTileInfo(UKFBB_FieldTile* T);

	FTileInfo& operator=(const UKFBB_FieldTile* T);
	bool operator==(const UKFBB_FieldTile* T) const;
	bool operator!=(const UKFBB_FieldTile* T) const;
	bool operator==(const FTileInfo T) const;
	bool operator!=(const FTileInfo T) const;

	FORCEINLINE bool IsValid() const { return TileIdx >= 0; }
	FORCEINLINE void Clear() { TileIdx = INDEX_NONE; }
};


USTRUCT(BlueprintType)
struct FTileDir
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 x;
	UPROPERTY(BlueprintReadWrite)
	int32 y;

	FTileDir() : x(0), y(0) {}
	FTileDir(short _x, short _y) : x(_x), y(_y) {}

	FORCEINLINE bool operator==(const FTileDir& V) const { return x == V.x && y == V.y; }
	FORCEINLINE bool operator!=(const FTileDir& V) const { return x != V.x || y != V.y; }

	static FTileDir ConvertToTileDir(FVector2D v);
	bool IsCardinalDir() { return !((FMath::Abs(x) != 0 && FMath::Abs(y) != 0) || (x == 0 && y == 0)); }
};
