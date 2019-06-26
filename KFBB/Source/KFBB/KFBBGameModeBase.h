// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "KFBBGameModeBase.generated.h"

class UUserWidget; 
class AKFBB_Field;
class AKFBB_PlayerPawn;
class AKFBB_CoachPC;
class AKFBB_TeamInfo;
class AKFBB_Ball;

namespace KFBBMatchState
{
	const FName EnteringMap = FName(TEXT("EnteringMap"));
	const FName WaitingToStart = FName(TEXT("WaitingToStart"));
	
	const FName InProgress = FName(TEXT("InProgress"));
	const FName InReplay = FName(TEXT("InReplay"));
	
	const FName WaitingPostMatch = FName(TEXT("WaitingPostMatch"));
	const FName LeavingMap = FName(TEXT("LeavingMap"));
	const FName Aborted = FName(TEXT("Aborted"));
}

/**
*
*/
UCLASS()
class KFBB_API AKFBBGameModeBase : public AGameMode
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
	AKFBBGameModeBase();

	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;

	// Ball Info
	virtual UClass* GetDefaultBallClass();
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<AKFBB_Ball> DefaultBallClass;

	// Player Info
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Classes")
	TSubclassOf<AKFBB_PlayerPawn> DefaultPlayerClass;

	//Team Info
	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<AKFBB_TeamInfo> TeamInfoClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Team")
	int32 NumTeams = 2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Team")
	TArray<AKFBB_TeamInfo*> Teams;

	virtual void SpawnTeams();
	virtual void SpawnTeamPlayers();
	virtual UClass* GetPawnClassByID(int PawnID);
	AKFBB_TeamInfo* GetTeamInfoByID(uint8 teamID);



	UFUNCTION(BlueprintPure)
	float GetRemainingGameTime() const;
	UFUNCTION(BlueprintPure)
	int GetFieldWidth() const;
	UFUNCTION(BlueprintPure)
	int GetFieldLength() const;
	UFUNCTION(BlueprintPure)
	FVector GetFieldTileLocation(int x, int y) const;


	virtual void ResolveCollision(AKFBB_PlayerPawn* PawnA, AKFBB_PlayerPawn* PawnB);


	
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual void RegisterCoach(AKFBB_CoachPC* PC, uint8 teamID);
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual void UnregisterCoach(AKFBB_CoachPC* PC);

	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual void RegisterTeamMember(AKFBB_PlayerPawn* P, uint8 teamID);
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual void UnregisterTeamMember(AKFBB_PlayerPawn* P);
};
