// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KFBB_Field.generated.h"

USTRUCT()
struct KFBB_API FFieldTile
{
	GENERATED_BODY()

public:
	FFieldTile();
	~FFieldTile();

	UPROPERTY()
	int idx;
	UPROPERTY()
	int x;
	UPROPERTY()
	int y;

	UPROPERTY()
	bool bEndZone;
	UPROPERTY()
	bool bWideOut;
	UPROPERTY()
	bool bScrimmageLine;

	UPROPERTY()
	class AKFBB_Field* Field;
	UPROPERTY()
	FVector TileLocation;

	UPROPERTY()
	UStaticMeshComponent* Comp;

	void Init(AKFBB_Field* inField, int inIdx, int inX, int inY);

	void DrawDebugTile();
};



UCLASS()
class KFBB_API AKFBB_Field : public AActor
{
	GENERATED_BODY()
	
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

	UPROPERTY(EditAnywhere)
	int Length;
	UPROPERTY(EditAnywhere)
	int Width;
	UPROPERTY(EditAnywhere)
	float TileSize;
	UPROPERTY(EditAnywhere)
	int EndZoneSize;
	UPROPERTY(EditAnywhere)
	int WideOutSize;
	
	UPROPERTY()
	FVector Origin;
	UPROPERTY()
	FVector TileStep;
	UPROPERTY()
	TArray<FFieldTile> Map;
	UPROPERTY(EditAnywhere)
	FStringAssetReference StaticMeshRef;

	int GetIndexByXY(int x, int y) const;

	UFUNCTION(BlueprintPure)
	int GetFieldWidth() const { return Width; }
	UFUNCTION(BlueprintPure)
	int GetFieldLength() const { return Length; }
	UFUNCTION(BlueprintPure)
	FVector GetFieldTileLocation(int x, int y) const;
};
