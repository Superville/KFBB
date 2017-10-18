// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KFBB_Field.generated.h"

struct FFieldTile
{
public:
	FFieldTile();
	~FFieldTile();

	int idx;
	int x;
	int y;

	bool bEndZone;
	bool bWideOut;
	bool bScrimmageLine;

	class AKFBB_Field* Field;
	FVector TileLocation;

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
	
	FVector Origin;
	FVector TileStep;
	TArray<FFieldTile> Map;
};
