// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"
#include "AbilitySystemInterface.h"
#include "KFBB_Ball.generated.h"


UCLASS()
class KFBB_API AKFBB_Ball : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

	void RegisterWithField();
	void RegisterWithTile(class UKFBB_FieldTile* Tile);

	bool bOnGround;
	float LastFumbleTime;


public:	
	// Sets default values for this actor's properties
	AKFBB_Ball();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BallSMC = nullptr;


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

	bool IsPossessed() const;

	bool CanBePickedUp() const;
	void UpdateCanBePickedUp();
	bool IsMoving() const;
	void StopMovement();
	void AdjustBallToTileCenter(float DeltaTime);
	void FumbleBall(UKFBB_FieldTile* DestTile);
	float TimeSinceLastFumble() const;
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTag CanBePickedUpTag;

	void DrawDebugCurrentTile() const;

	//test
	void Test();
};
