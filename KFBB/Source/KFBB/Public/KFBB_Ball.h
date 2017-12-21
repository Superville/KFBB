// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KFBB_Ball.generated.h"

UCLASS()
class KFBB_API AKFBB_Ball : public AActor
{
	GENERATED_BODY()

	void RegisterWithField();
	void RegisterWithTile(class UKFBB_FieldTile* Tile);

public:	
	// Sets default values for this actor's properties
	AKFBB_Ball();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(BlueprintReadOnly)
	class AKFBB_Field *Field;
	UPROPERTY(BlueprintReadOnly)
	class UKFBB_FieldTile* CurrentTile;

	UPROPERTY(BlueprintReadOnly)
	class AKFBB_PlayerPawn* OwningPlayer;

	void RegisterWithPlayer(class AKFBB_PlayerPawn* P);
	void UnRegisterWithPlayer();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
	void DrawDebugCurrentTile() const;
};
