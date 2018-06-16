// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "KFBB_AIController.generated.h"

/**
 * 
 */
UCLASS()
class KFBB_API AKFBB_AIController : public AAIController
{
	GENERATED_BODY()

	TArray<class UKFBB_FieldTile*> PathToDestTile;

public:
	AKFBB_AIController(const FObjectInitializer& ObjectInitializer);

	class AKFBB_PlayerPawn* MyPlayerPawn;

	virtual void SetPawn(APawn* InPawn) override;

	/** Called on completing current movement request */
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
	
	UPROPERTY(BlueprintReadonly)
	class UKFBB_FieldTile* DestinationTile;

	class AKFBB_Field* GetField();
	void ClearPathing();
	bool GeneratePathToTile(class UKFBB_FieldTile* DestTile);
	float GetPathGlobalCost(class UKFBB_FieldTile* curr, class UKFBB_FieldTile* next) const;
	float GetPathHeuristicCost(class UKFBB_FieldTile* dest, class UKFBB_FieldTile* next) const;
	class UKFBB_FieldTile* GetNextTileOnPath() const;
	void DrawDebugPath() const;

};
