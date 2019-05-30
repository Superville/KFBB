// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KFBB_CoachPC.generated.h"

class AKFBB_PlayerPawn;
class AKFBB_Field;
class UKFBB_Field_Tile;
class AKFBB_Ball;

/**
 * 
 */
UCLASS()
class KFBB_API AKFBB_CoachPC : public APlayerController
{
	GENERATED_BODY()

	void DrawDebugTouchedTile(UKFBB_FieldTile* t);

	virtual void SetSelectedPlayer(AKFBB_PlayerPawn* p);
	virtual void SetSelectedTile(UKFBB_FieldTile* t);
	virtual void SetDestinationTile(UKFBB_FieldTile* t);
	virtual void ConfirmCommand();

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
	void SpawnBallOnTile();
	void SpawnBallOnTile(int x, int y);

	AKFBB_PlayerPawn* GetSelectedPlayer() const;

	UFUNCTION(BlueprintCallable)
	void ClearTileSelection();

	UPROPERTY(BlueprintReadonly)
	AKFBB_Field* Field;

	UPROPERTY(BlueprintReadonly)
	UKFBB_FieldTile* SelectedTile;
	UPROPERTY(BlueprintReadonly)
	UKFBB_FieldTile* DestinationTile;
	UPROPERTY(BlueprintReadonly)
	AKFBB_PlayerPawn* SelectedPlayer;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AKFBB_PlayerPawn> PlayerClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AKFBB_Ball> BallClass;


	UPROPERTY(EditDefaultsOnly)
	float PlayerSpawnOffsetZ;
	
};
