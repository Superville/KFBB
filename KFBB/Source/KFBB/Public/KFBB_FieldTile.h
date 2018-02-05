// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "KFBB_FieldTile.generated.h"

/**
 * 
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class KFBB_API UKFBB_FieldTile : public UStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	int idx;
	UPROPERTY(BlueprintReadOnly)
	int x;
	UPROPERTY(BlueprintReadOnly)
	int y;

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
	class AKFBB_Field* Field;
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

	UFUNCTION(BlueprintCallable)
	AKFBB_PlayerPawn* GetPlayer() const;
	UFUNCTION(BlueprintCallable)
	AKFBB_Ball* GetBall() const;



	UFUNCTION(BlueprintCallable)
	void DrawDebugTile() const;
	void DrawDebugTile(FVector offset) const;
	void DrawDebugTileOverride(FVector offset, float scale, FColor c) const;
	FColor GetDebugColor() const;
};
