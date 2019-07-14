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

	

public:
	AKFBB_AIController(const FObjectInitializer& ObjectInitializer);

	AKFBB_PlayerPawn* MyPlayerPawn;

	virtual void SetPawn(APawn* InPawn) override;

	AKFBB_Field* GetField() const;
	
	UPROPERTY(BlueprintReadonly)
	bool bAbortGridMove = false;

	UFUNCTION(BlueprintPure, Category = "KFBB | AI")
	virtual UKFBB_FieldTile* GetDestinationTile();
	virtual bool MarkDestinationTile(UKFBB_FieldTile* DestTile);

	UFUNCTION(BlueprintPure, Category = "KFBB | AI")
	virtual TArray<UKFBB_FieldTile*> GetDestinationPath();
	virtual bool MarkDestinationPath(TArray<UKFBB_FieldTile*>& ProvidedPath);

	virtual void ClearDestination();
	virtual bool ConfirmMoveToDestinationTile();
	
	void ClearPathing(TArray<UKFBB_FieldTile*>& out_PathList);
	bool AddToPath(UKFBB_FieldTile* StartTile, UKFBB_FieldTile* DestTile, TArray<UKFBB_FieldTile*>& out_PathList);
	bool GeneratePathToTile(UKFBB_FieldTile* StartTile, UKFBB_FieldTile* DestTile, TArray<UKFBB_FieldTile*>& out_PathList, int MaxPathLength = 0);
	float GetPathGlobalCost(UKFBB_FieldTile* curr, UKFBB_FieldTile* next) const;
	float GetPathHeuristicCost(UKFBB_FieldTile* dest, UKFBB_FieldTile* next) const;
	bool CanMoveThruTile(UKFBB_FieldTile* tile) const;

	FName PathSetName = FName("bPathSet");

	uint8 GetTeamID() const;
};
