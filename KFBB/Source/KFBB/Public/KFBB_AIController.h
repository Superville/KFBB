// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "KFBB_AIController.generated.h"

class AKFBB_PlayerPawn;
class UKFBB_FieldTile;
class AKFBB_Field;

/**
 * 
 */
UCLASS()
class KFBB_API AKFBB_AIController : public AAIController
{
	GENERATED_BODY()

	TArray<UKFBB_FieldTile*> PathToDestTile;

public:
	AKFBB_AIController(const FObjectInitializer& ObjectInitializer);

	AKFBB_PlayerPawn* MyPlayerPawn;

	virtual void SetPawn(APawn* InPawn) override;

	/** Called on completing current movement request */
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	
	UPROPERTY(BlueprintReadonly)
	UKFBB_FieldTile* DestinationTile;

	AKFBB_Field* GetField();
	void ClearPathing();
	bool GeneratePathToTile(UKFBB_FieldTile* DestTile);
	float GetPathGlobalCost(UKFBB_FieldTile* curr, UKFBB_FieldTile* next) const;
	float GetPathHeuristicCost(UKFBB_FieldTile* dest, UKFBB_FieldTile* next) const;

	
	bool SetBlackboardTilePath();
	void ClearBlackboardTilePath();

	FName PathSetName = FName("bPathSet");
	FName TilePathName = FName("TilePath");
	
	UKFBB_FieldTile* GetNextTileOnPath() const;
	
	UFUNCTION(BlueprintCallable, Category = "Pathing")
	void UpdateTilePath();
	
	void DrawDebugPath() const;



};
