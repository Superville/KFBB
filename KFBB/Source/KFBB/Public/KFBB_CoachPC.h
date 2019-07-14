// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KFBB_CoachPC.generated.h"

class AKFBB_PlayerPawn;
class AKFBB_AIController;
class AKFBB_Field;
class UKFBB_Field_Tile;
class AKFBB_Ball;

UENUM(BlueprintType)
namespace EDebugTile
{
	enum Type
	{
		Show_None			UMETA(DisplayName = "None"),
		Show_TileIndex		UMETA(DisplayName = "TileIndex"),
		Show_TileXY			UMETA(DisplayName = "TileXY"),
		Show_TileTeamID		UMETA(DisplayName = "TileTeam"),
	};
}

/**
 * 
 */
UCLASS()
class KFBB_API AKFBB_CoachPC : public APlayerController
{
	GENERATED_BODY()

	void DrawDebugTouchedTile(UKFBB_FieldTile* t);

	virtual void SetSelectedPlayer(AKFBB_PlayerPawn* p);
	virtual void ClearSelectedPlayer();

	UKFBB_FieldTile* SelectedTile;
	virtual void MarkSelectedTile(UKFBB_FieldTile* Tile);
	virtual void SetSelectedTile(UKFBB_FieldTile* Tile);
	virtual void ClearSelectedTile();
	UPROPERTY(ReplicatedUsing = OnRep_SelectedTile)
	int32 RepSelectedTile;
	UFUNCTION()
	void OnRep_SelectedTile();

	TArray<UKFBB_FieldTile*> SelectedTileList;
	virtual void MarkSelectedTileList(TArray<UKFBB_FieldTile*>& ProvidedPath);
	virtual void SetSelectedTileList(TArray<UKFBB_FieldTile*>& TileList);
	virtual void ClearSelectedTileList();
	UPROPERTY(ReplicatedUsing = OnRep_SelectedTileList)
	TArray<int32> RepSelectedTileList;
	UFUNCTION()
	void OnRep_SelectedTileList();
	
	UKFBB_FieldTile* DestinationTile;
	virtual void SetDestinationTile(UKFBB_FieldTile* t);
	virtual void ClearDestinationTile();
	UPROPERTY(ReplicatedUsing = OnRep_DestinationTile)
	int32 RepDestinationTile;
	UFUNCTION()
	void OnRep_DestinationTile();


	virtual void ConfirmCommand();

public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	virtual void PlayerTouchScreen();
	UFUNCTION(Server, Reliable, WithValidation, Category = "KFBB | Input")
	void ServerTouchScreen(int TileIdx, bool bBeginDrag);
	UFUNCTION(BlueprintCallable)
	virtual void PlayerUntouchScreen();
	UFUNCTION(Server, Reliable, WithValidation, Category = "KFBB | Input")
	void ServerPlayerUntouchScreen(int TileIdx);
	
	virtual void SetDragInfo(UKFBB_FieldTile* Tile);
	virtual void ClearDragInfo();
	virtual bool BeginDragTouch(UKFBB_FieldTile* Tile);
	virtual void EndDragTouch(UKFBB_FieldTile* Tile);
	virtual void CheckDragPath();
	UFUNCTION(Server, Reliable, WithValidation, Category = "KFBB | Input")
	void ServerUpdateDragPath(int TileIdx);

	virtual void AddToPath(UKFBB_FieldTile* Tile);
	UKFBB_FieldTile* StartDragTile = nullptr;
	bool bIsDraggingPath = false;
	bool bIsDragging = false;

	UFUNCTION(BlueprintCallable)
	UKFBB_FieldTile* GetTileUnderMouse( float ReqDistFromCenterScale = 0.f);
	float MouseUnderTileScalar = 0.75f;
	FVector MouseWorldLoc;
	FVector MouseWorldDir;
	virtual void UpdateDisplayTileUnderMouse();
	UKFBB_FieldTile* DisplayTileUnderMouse = nullptr;

	UFUNCTION(BlueprintCallable, Category = "KFBB")
	void SpawnPlayerOnTileUnderMouse(uint8 teamID = 0);
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "KFBB")
	void SpawnPlayerOnTile(int x, int y, uint8 teamID);
	void SpawnPlayerOnTile_Implementation(int x, int y, uint8 teamID);

	UFUNCTION(BlueprintCallable, Category = "KFBB")
	void SpawnBallOnTileUnderMouse();
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "KFBB")
	void SpawnBallOnTile(int x, int y);
	void SpawnBallOnTile_Implementation(int x, int y);

	AKFBB_PlayerPawn* GetSelectedPlayer() const;

	UFUNCTION(BlueprintCallable, Category = "KFBB | Input", meta=(DisplayName="ClearTileSelection"))
	void BP_ClearTileSelection(bool bClearAI = true);
	UFUNCTION(Server, Reliable, WithValidation, Category = "KFBB | Input")
	void ServerClearTileSelection(bool bClearAI);
	void ClearTileSelection(bool bClearAI = true);

	UPROPERTY(BlueprintReadonly)
	AKFBB_Field* Field;


	
	UPROPERTY(BlueprintReadonly)
	AKFBB_AIController* SelectedAI;
	UPROPERTY(BlueprintReadonly, Replicated)
	AKFBB_PlayerPawn* SelectedPlayer;
	UPROPERTY(BlueprintReadonly, Replicated)
	AKFBB_PlayerPawn* PrevSelectedPlayer;
	virtual void DrawSelectedPlayer();
	
	UPROPERTY(BlueprintReadOnly)
	TArray<UKFBB_FieldTile*> PotentialMoveList;
	virtual void UpdatePotentialMoveList();
	virtual void DrawPotentialMoveList();

	UPROPERTY(Replicated, BlueprintReadonly, Category = "KFBB")
	bool bReadyToStart = false;
	virtual bool IsReadyToStart();
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "KFBB")
	void ToggleReadyToStart();
	virtual void ToggleReadyToStart_Implementation();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "KFBB")
	uint8 GetTeamID() const;
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	void SetTeamID(uint8 teamID);
	uint8 TeamID = 255;


	//debug
	virtual void DrawDebug(float DeltaTime);
	bool bDebugHighlightSelectedTile = false;
	float DebugFlashSelectedTileTimer;

	UFUNCTION(BlueprintCallable, Category = "KFBB")
	void NextTileDebugState();
	void DrawTileDebug(float DeltaTime);
	TEnumAsByte<EDebugTile::Type> TileDebugState = EDebugTile::Show_None;
};
