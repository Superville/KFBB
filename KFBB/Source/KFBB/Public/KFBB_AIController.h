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
	UPROPERTY(BlueprintReadonly)
	UKFBB_FieldTile* DestinationTile;
	UPROPERTY(BlueprintReadonly)
	TArray<UKFBB_FieldTile*> PathToDestTile;

	virtual bool SetDestinationTile(UKFBB_FieldTile* DestTile);
	virtual bool SetDestinationTile(TArray<UKFBB_FieldTile*>& ProvidedPath);
	virtual void ClearDestinationTile();

	void ClearPathing();
	bool GeneratePathToTile(UKFBB_FieldTile* DestTile);
	float GetPathGlobalCost(UKFBB_FieldTile* curr, UKFBB_FieldTile* next) const;
	float GetPathHeuristicCost(UKFBB_FieldTile* dest, UKFBB_FieldTile* next) const;
	bool CanMoveThruTile(UKFBB_FieldTile* tile) const;

	FName PathSetName = FName("bPathSet");
	
	void DrawDebugPath() const;
};
