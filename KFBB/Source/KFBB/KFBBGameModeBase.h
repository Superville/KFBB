// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KFBBGameModeBase.generated.h"

class UUserWidget; 
class AKFBB_Field;
class AKFBB_PlayerPawn;

/**
*
*/
UCLASS()
class KFBB_API AKFBBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
		
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Meta = (BlueprintProtected = "true"))
	float GameDurationInSeconds;
	UPROPERTY()
	float CurrentGameTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (BlueprintProtected = "true"))
	TSubclassOf<UUserWidget> HUDWidgetClass;
	UPROPERTY()
	UUserWidget* CurrentWidget;

	UPROPERTY()
	AKFBB_Field* Field;

public:
	UFUNCTION(BlueprintPure)
	float GetRemainingGameTime() const;
	UFUNCTION(BlueprintPure)
	int GetFieldWidth() const;
	UFUNCTION(BlueprintPure)
	int GetFieldLength() const;
	UFUNCTION(BlueprintPure)
	FVector GetFieldTileLocation(int x, int y) const;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void ResolveCollision(AKFBB_PlayerPawn* PawnA, AKFBB_PlayerPawn* PawnB);
};
