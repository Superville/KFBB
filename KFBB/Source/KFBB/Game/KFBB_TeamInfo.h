// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "KFBB_TeamInfo.generated.h"

class AKFBB_AIController;
class AKFBB_PlayerPawn;
class AKFBBGameModeBase;
class UKFBB_FieldTile;
class AKFBB_Field;

USTRUCT(BlueprintType)
struct FTeamMember
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY()
	AKFBB_PlayerPawn* Player = nullptr;
	UPROPERTY()
	AKFBB_AIController* AI = nullptr;

	UPROPERTY()
	int32 TypeID = -1; // Identifies the pawn class/etc.
	
	UPROPERTY()
	int32 SpawnTileX = -1;
	UPROPERTY()
	int32 SpawnTileY = -1;
	UPROPERTY()
	UKFBB_FieldTile* SpawnTile = nullptr;
};

/**
 * 
 */
UCLASS()
class KFBB_API AKFBB_TeamInfo : public AInfo
{
	GENERATED_BODY()

	AKFBBGameModeBase* KFGM = nullptr;
	AKFBB_Field* Field = nullptr;

	int32 GetTeamMemberIndex(AKFBB_PlayerPawn* P);
	int32 GetCoachIndex(AKFBB_CoachPC* C);
	
	void SetTeamID(uint8 teamID);
	uint8 TeamID = 255;
public:
	uint8 GetTeamID() const;
	
	UPROPERTY(BlueprintReadOnly, Category = "KFBB")
	TArray<AKFBB_CoachPC*> CoachList;
	UPROPERTY(BlueprintReadOnly, Category = "KFBB")
	TArray<FTeamMember> MemberList;

	virtual void Init(AKFBBGameModeBase* InGameMode, int32 InTeamID, AKFBB_Field* InField, FString OptionsString);
	virtual void SetupPlayers(FString OptionsString);
	virtual void ParsePlayerString(FString PlayerString);
	virtual void SpawnPlayers();

	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual void RegisterCoach(AKFBB_CoachPC* PC);
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual void UnregisterCoach(AKFBB_CoachPC* PC);
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual bool IsCoach(AKFBB_CoachPC* PC);

	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual void RegisterTeamMember(AKFBB_PlayerPawn* P);
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual void UnregisterTeamMember(AKFBB_PlayerPawn* P);
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	virtual bool IsTeamMember(AKFBB_PlayerPawn* P);

	friend class AKFBBGameModeBase;
};
