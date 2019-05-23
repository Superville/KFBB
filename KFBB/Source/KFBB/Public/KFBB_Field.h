// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KFBB_Field.generated.h"

USTRUCT()
struct FTileDir
{
	GENERATED_BODY()

	FTileDir() : x(0), y(0) {}
	FTileDir(short _x, short _y) : x(_x), y(_y) {}
	short x;
	short y;

	FORCEINLINE bool operator==(const FTileDir& V) const { return x == V.x && y == V.y; }
	FORCEINLINE bool operator!=(const FTileDir& V) const { return x != V.x || y != V.y; }
};

UCLASS()
class KFBB_API AKFBB_Field : public AActor
{
	GENERATED_BODY()

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
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Searches the map for a KFBB_Field actor and assigns it to the pointer ref
	static bool AssignFieldActor(AActor* src, AKFBB_Field*& ptrField);

	TArray<class UKFBB_FieldTile*> Tiles;

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
	class USceneComponent* Root;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* TileMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* Mat_Field;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterial* Mat_EndZone;
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
	int GetFieldWidth() const { return Width; }
	UFUNCTION(BlueprintPure)
	int GetFieldLength() const { return Length; }
	UFUNCTION(BlueprintPure)
	FVector GetFieldTileLocation(int x, int y) const;

	TArray<struct FTileDir> TileDirections;
	FTileDir GetScatterDirection(short centerX = 0, short centerY = 0, int cone = 4);
	class UKFBB_FieldTile* GetAdjacentTile(class UKFBB_FieldTile* tile, FTileDir dir);
};


