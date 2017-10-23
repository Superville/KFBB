// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KFBB_PlayerPawn.h"
#include "KFBB_FieldTile.h"
#include "GameFramework/PlayerController.h"
#include "KFBB_CoachPC.generated.h"

/**
 * 
 */
UCLASS()
class KFBB_API AKFBB_CoachPC : public APlayerController
{
	GENERATED_BODY()

	void DrawDebugTouchedTile(UKFBB_FieldTile* t);

	void SetDestinationTile(UKFBB_FieldTile* t);
	bool TryMovePawnToDestination();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void PlayerTouchScreen();

	UFUNCTION(BlueprintCallable)
	UKFBB_FieldTile* GetTileUnderMouse();

	UFUNCTION(BlueprintCallable)
	void SpawnPlayerOnTile();
	void SpawnPlayerOnTile(int x, int y);

	UFUNCTION(BlueprintCallable)
	void ClearTileSelection();
		
	UPROPERTY(BlueprintReadonly)
	class AKFBB_Field* Field;

	UPROPERTY(BlueprintReadonly)
	class UKFBB_FieldTile* SelectedTile;
	UPROPERTY(BlueprintReadonly)
	class UKFBB_FieldTile* DestinationTile;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AKFBB_PlayerPawn> PlayerClass;

	UPROPERTY(EditDefaultsOnly)
	float PlayerSpawnOffsetZ;
	
};
