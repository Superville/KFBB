// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "KFBB_FieldTile.generated.h"

class AKFBB_Field;

/**
 * 
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class KFBB_API UKFBB_FieldTile : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	int TileIdx;
	UPROPERTY(BlueprintReadOnly)
	int TileX;
	UPROPERTY(BlueprintReadOnly)
	int TileY;

	UPROPERTY(BlueprintReadOnly)
	bool bEndZone;
	UPROPERTY(BlueprintReadOnly)
	bool bWideOut;
	UPROPERTY(BlueprintReadOnly)
	bool bScrimmageLine;

	UPROPERTY()
	bool bPathVisited;
	UPROPERTY()
	float pathHeuristicCost;
	UPROPERTY()
	float pathGlobalCost;
	UPROPERTY()
	float pathTotalCost;
	UPROPERTY()
	UKFBB_FieldTile* pathPrevTile;

	UPROPERTY(BlueprintReadOnly)
	AKFBB_Field* Field;
	UPROPERTY(BlueprintReadOnly)
	FVector TileLocation;
	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> RegisteredActors;

	UFUNCTION(BlueprintCallable)
	void Init(AKFBB_Field* inField, int inIdx);

	UFUNCTION(BlueprintCallable)
	bool RegisterActor(AActor* a);
	UFUNCTION(BlueprintCallable)
	bool UnRegisterActor(AActor* a);

	UFUNCTION(BlueprintCallable)
	bool HasPlayer() const;
	UFUNCTION(BlueprintCallable)
	bool HasBall() const;

	FVector GetTileSize2D() const;

	UFUNCTION(BlueprintCallable)
	AKFBB_PlayerPawn* GetPlayer() const;
	UFUNCTION(BlueprintCallable)
	AKFBB_Ball* GetBall() const;

	UFUNCTION(BlueprintCallable)
	virtual void DrawCooldownTimer(float Duration, float TimeRemaining, FColor DrawColor);

	UFUNCTION(BlueprintCallable)
	void DrawDebugTile() const;
	void DrawDebugTile(FVector offset) const;
	void DrawDebugTileOverride(FVector offset, float scale, FColor c) const;
	FColor GetDebugColor() const;
};
