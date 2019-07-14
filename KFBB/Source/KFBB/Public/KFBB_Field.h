// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KFBB_Field.generated.h"

class USceneComponent;
class UKFBB_FieldTile;
class UMaterialInstance;
class UMaterial;

USTRUCT()
struct FTileDir
{
	GENERATED_BODY()

	short x;
	short y;

	FTileDir() : x(0), y(0) {}
	FTileDir(short _x, short _y) : x(_x), y(_y) {}

	FORCEINLINE bool operator==(const FTileDir& V) const { return x == V.x && y == V.y; }
	FORCEINLINE bool operator!=(const FTileDir& V) const { return x != V.x || y != V.y; }

	static FTileDir ConvertToTileDir(FVector2D v);
	bool IsCardinalDir() { return !((FMath::Abs(x) != 0 && FMath::Abs(y) != 0) || (x == 0 && y == 0)); }
};

UCLASS()
class KFBB_API AKFBB_Field : public AActor
{
	GENERATED_BODY()

	bool bFieldInitialized = false;

	AKFBB_Field(const FObjectInitializer& ObjectInitializer);
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// Sets default values for this actor's properties
	AKFBB_Field();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

public:
	virtual void Init();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	

	// Searches the map for a KFBB_Field actor and assigns it to the pointer ref
	static bool AssignFieldActor(AActor* src, AKFBB_Field*& ptrField);

	TArray<UKFBB_FieldTile*> Tiles;
	TArray<UKFBB_FieldTile*> EndZoneTiles_Team0;
	TArray<UKFBB_FieldTile*> EndZoneTiles_Team1;
	TArray<UKFBB_FieldTile*> Tiles_Team0;
	TArray<UKFBB_FieldTile*> Tiles_Team1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Length;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int Width;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSize;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileHeight;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int EndZoneSize;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int WideOutSize;

	UPROPERTY()
	USceneComponent* Root;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* TileMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* Mat_Field;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* Mat_EndZone;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInstance* MatInst_EndZone_Blue;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInstance* MatInst_EndZone_Red;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* Mat_WideOut;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* Mat_Scrimmage;

	UPROPERTY(BlueprintReadOnly)
	FVector Origin;
	UPROPERTY(BlueprintReadOnly)
	FVector TileStep;

	UFUNCTION(BlueprintPure)
	int GetIndexByXY(int x, int y) const;
	UFUNCTION(BlueprintPure)
	int GetXByIndex(int idx) const;
	UFUNCTION(BlueprintPure)
	int GetYByIndex(int idx) const;
	UFUNCTION(BlueprintPure)
	UKFBB_FieldTile* GetTileByXY(int x, int y) const;
	UKFBB_FieldTile* GetTileByIndex(int idx) const;
	void ConvertArrayIndexToTile(TArray<int32> IndexArray, TArray<UKFBB_FieldTile*>& TileArray);
	void ConvertArrayTileToIndex(TArray<UKFBB_FieldTile*> TileArray, TArray<int32>& IndexArray);

	UFUNCTION(BlueprintPure)
	int GetFieldWidth() const { return Width; }
	UFUNCTION(BlueprintPure)
	int GetFieldLength() const { return Length; }
	UFUNCTION(BlueprintPure)
	FVector GetFieldTileLocation(int x, int y) const;

	UPROPERTY(EditDefaultsOnly)
	float PlayerSpawnOffsetZ;

	UFUNCTION()
	void TeamColorTileMaterials(int TeamIdx);

	TArray<struct FTileDir> TileDirections;
	FTileDir GetScatterDirection(short centerX = 0, short centerY = 0, int cone = 4);
	UKFBB_FieldTile* GetAdjacentTile(UKFBB_FieldTile* tile, FTileDir dir);
	static bool AreAdjacentTiles(UKFBB_FieldTile* a, UKFBB_FieldTile* b);
	FTileDir GetTileDir(UKFBB_FieldTile* a, UKFBB_FieldTile* b);

	TArray<UKFBB_FieldTile*> GetListOfTilesInRange(UKFBB_FieldTile* StartTile, int Range);
	TArray<FVector> GetExternalEdgeVerts(TArray<UKFBB_FieldTile*>& TileList);
};


