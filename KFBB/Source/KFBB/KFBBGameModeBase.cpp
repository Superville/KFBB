// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBBGameModeBase.h"

// Engine Includes
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"

// KFBB Includes
#include "KFBB_Field.h"
#include "KFBB_PlayerPawn.h"
#include "KFBB_CoachPC.h"
#include "Game/KFBB_TeamInfo.h"


void AKFBBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentGameTimer = GameDurationInSeconds;

	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}

	AKFBB_Field::AssignFieldActor(this, Field);

	CreateTeamInfos();
}

void AKFBBGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentGameTimer -= DeltaTime;
}

float AKFBBGameModeBase::GetRemainingGameTime() const
{
	return CurrentGameTimer;
}

int AKFBBGameModeBase::GetFieldWidth() const
{
	if (Field == nullptr)
		return -1;
	return Field->Width;
}

int AKFBBGameModeBase::GetFieldLength() const
{
	if (Field == nullptr)
		return -1;
	return Field->Length;
}

FVector AKFBBGameModeBase::GetFieldTileLocation(int x, int y) const
{
	if (Field == nullptr)
		return FVector::ZeroVector;

	return Field->GetFieldTileLocation(x, y);
}

void AKFBBGameModeBase::ResolveCollision(AKFBB_PlayerPawn* PawnA, AKFBB_PlayerPawn* PawnB)
{
	// PawnA = player already occupying tile
	// PawnB = player moving onto the tile, PawnB->PreviousTile will give direction of movement

	auto KnockDirA = Field->GetTileDir(PawnB->PreviousTile, PawnB->CurrentTile);
	auto KnockDirB = Field->GetTileDir(PawnB->CurrentTile, PawnB->PreviousTile);
	
// 	KnockDirA.x = 0;
// 	KnockDirB.y = 0;

	PawnA->KnockDown(KnockDirA);
	PawnB->KnockDown(KnockDirB);
}

void AKFBBGameModeBase::CreateTeamInfos()
{
	if (!TeamInfoClass) { return; }

	for (int i = 0; i < NumTeams; i++)
	{
		auto TeamInfo = GetWorld()->SpawnActor<AKFBB_TeamInfo>(TeamInfoClass);
		if (TeamInfo)
		{
			int32 TeamID = Teams.Add(TeamInfo);
			TeamInfo->SetTeamID(TeamID);
		}
	}
}

AKFBB_TeamInfo* AKFBBGameModeBase::GetTeamInfoByID(uint8 teamID)
{
	if (!Teams.IsValidIndex(teamID)) { return nullptr; }
	return Teams[teamID];
}

void AKFBBGameModeBase::RegisterCoach(AKFBB_CoachPC* PC, uint8 teamID)
{
	if (!PC || !Teams.IsValidIndex(teamID)) { return; }
	Teams[teamID]->RegisterCoach(PC);
}

void AKFBBGameModeBase::UnregisterCoach(AKFBB_CoachPC* PC)
{
	if (!PC) { return; }
	uint8 teamID = PC->GetTeamID();
	if (!Teams.IsValidIndex(teamID)) { return; }
	Teams[teamID]->UnregisterCoach(PC);
}

void AKFBBGameModeBase::RegisterTeamMember(AKFBB_PlayerPawn* P, uint8 teamID)
{
	if (!P || !Teams.IsValidIndex(teamID)) { return; }
	Teams[teamID]->RegisterTeamMember(P);
}

void AKFBBGameModeBase::UnregisterTeamMember(AKFBB_PlayerPawn* P)
{
	if (!P) { return; }
	uint8 teamID = P->GetTeamID();
	if (!Teams.IsValidIndex(teamID)) { return; }
	Teams[teamID]->UnregisterTeamMember(P);
}
