// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KFBB_PlayerPawn.generated.h"

UCLASS()
class KFBB_API AKFBB_PlayerPawn : public ACharacter
{
	GENERATED_BODY()

	void RegisterWithField();
	void RegisterWithTile(class UKFBB_FieldTile* Tile);

	void DrawDebugCurrentTile();

	void ClearCooldownTimer();
	void ResetCooldownTimer();

public:
	// Sets default values for this pawn's properties
	AKFBB_PlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadOnly)
	class AKFBB_CoachPC* Coach;
	UPROPERTY(BlueprintReadOnly)
	class AKFBB_Field *Field;
	UPROPERTY(BlueprintReadOnly)
	class UKFBB_FieldTile* CurrentTile;


	UPROPERTY(BlueprintReadOnly)
	float CooldownTimer;

	UFUNCTION(BlueprintCallable)
	bool IsPlayerOnCooldown();

	UFUNCTION(BlueprintCallable)
	bool CanAcceptCommand();


	void NotifyCommandFailed();
	void NotifyReachedDestination();	
};
