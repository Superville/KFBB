// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_TeamInfo.h"

// Engine Includes
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// KFBB Includes
#include "KFBB_PlayerPawn.h"
#include "KFBB_AIController.h"
#include "KFBB_CoachPC.h"
#include "KFBBGameModeBase.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"

void AKFBB_TeamInfo::Init(AKFBBGameModeBase* InGameMode, int32 InTeamID, AKFBB_Field* InField, FString OptionsString)
{
	KFGM = InGameMode;
	TeamID = InTeamID;
	Field = InField;

	SetupPlayers(OptionsString);
}

// TeamString Format "PlayerString/PlayerString/..."
void AKFBB_TeamInfo::SetupPlayers(FString OptionsString)
{
	if (!Field) { return; }

	FString OptionID = FString::Printf(TEXT("Team%d"), TeamID);
	if (!UGameplayStatics::HasOption(OptionsString, OptionID))
	{
		// ERROR!
//		OptionsString = FString::Printf(TEXT("%s=0,0,0,0,0"), *OptionID);
//		OptionsString = FString::Printf(TEXT("%s=0,0"), *OptionID);

		UE_LOG(LogTemp, Error, TEXT("AFantasyBallTeamInfo::SetupPlayers - No Option in OptionsString for %s"), *OptionID);
		return;
	}

	auto TeamString = UGameplayStatics::ParseOption(OptionsString, OptionID);

	TArray<FString> PlayerStrings;
	int SlashPos = INDEX_NONE;
	while (TeamString.FindChar(TCHAR('/'), SlashPos))
	{
		PlayerStrings.Add(TeamString.Left(SlashPos));
		TeamString = TeamString.Mid(SlashPos + 1);
	}
	PlayerStrings.Add(TeamString);

	for (int i = 0; i < PlayerStrings.Num(); i++)
	{
		ParsePlayerString(PlayerStrings[i]);
	}
}

// PlayerString Format "ID,FieldPctX,FieldPctY"
// FieldPct relative to teams side of the field
void AKFBB_TeamInfo::ParsePlayerString(FString PlayerString)
{
	int32 TypeID = INDEX_NONE;
	float FieldPctX = -1;
	float FieldPctY = -1;

	TArray<FString> PlayerParams;
	int32 CommaPos = INDEX_NONE;
	while (PlayerString.FindChar(TCHAR(','), CommaPos))
	{
		PlayerParams.Add(PlayerString.Left(CommaPos));
		PlayerString = PlayerString.Mid(CommaPos + 1);
	}
	PlayerParams.Add(PlayerString);

	for (int i = 0; i < PlayerParams.Num(); i++)
	{
		if (i == 0) { TypeID = FCString::Atoi(*PlayerParams[i]); }
		else if (i == 1) { FieldPctX = FCString::Atof(*PlayerParams[i]); }
		else if (i == 2) { FieldPctY = FCString::Atof(*PlayerParams[i]); }
	}
	FieldPctX = FMath::Clamp(FieldPctX, 0.f, 1.f);
	FieldPctY = FMath::Clamp(FieldPctY, 0.f, 1.f);

	if (TeamID == 1)
	{
		FieldPctX = 1.f - FieldPctX;
		FieldPctY = 1.f - FieldPctY;
	}

	int32 FieldHalfLength = Field->Length / 2;
	int32 TileX = FMath::FloorToInt((Field->Width - 1) * FieldPctX);
	int32 TileY = FMath::FloorToInt((FieldHalfLength - 1) * FieldPctY) + ((TeamID == 0) ? FieldHalfLength : 0 );

	FTeamMember TM;
	TM.TypeID = TypeID;
	TM.SpawnTileX = TileX;
	TM.SpawnTileY = TileY;
	TM.SpawnTile = Field->GetTileByXY(TileX, TileY);
	MemberList.Add(TM);

	//debug
// 	if (TM.SpawnTile)
// 	{
// 		DrawDebugSphere(GetWorld(), TM.SpawnTile->TileLocation + FVector(0, 0, 10), 16.f, 32, TeamID == 0 ? FColor::Blue : FColor::Red, true);
// 	}
}

void AKFBB_TeamInfo::SpawnPlayers()
{
	if (!KFGM || !Field) { return; }

	for (int i = 0; i < MemberList.Num(); i++)
	{
		FTeamMember& TM = MemberList[i];
		if (!TM.SpawnTile)
		{
			TM.SpawnTile = Field->GetTileByXY(TM.SpawnTileX, TM.SpawnTileY);
		}

		if (!TM.SpawnTile) { continue; }

		auto PawnClass = KFGM->GetPawnClassByID(TM.TypeID);

		FVector StartLocation = TM.SpawnTile->TileLocation + FVector(0, 0, Field->PlayerSpawnOffsetZ);
		FRotator StartRotation(ForceInit);
		StartRotation.Yaw = Field->GetActorRotation().Yaw + ((TeamID == 0) ? 180.f : 0.f);
		FTransform SpawnTransform = FTransform(StartRotation, StartLocation);

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = Instigator;
		SpawnInfo.ObjectFlags |= RF_Transient; // We never want to save default player pawns or controllers into a map
		auto ResultPawn = GetWorld()->SpawnActor<ACharacter>(PawnClass, SpawnTransform, SpawnInfo);
		if (!ResultPawn)
		{
			// error
			continue;
		}

		auto KFP = Cast<AKFBB_PlayerPawn>(ResultPawn);
		if (KFP)
		{
			TM.Player = KFP;
			KFP->SetTeamID(TeamID);
		}
	}
}

void AKFBB_TeamInfo::RegisterCoach(AKFBB_CoachPC* PC)
{
	if (!PC) { return; }
	CoachList.AddUnique(PC);
	PC->TeamID = TeamID;
}

void AKFBB_TeamInfo::UnregisterCoach(AKFBB_CoachPC* PC)
{
	int32 i = GetCoachIndex(PC);
	if (i >= 0)
	{
		CoachList.RemoveAt(i);
	}
}

bool AKFBB_TeamInfo::IsCoach(AKFBB_CoachPC* PC)
{
	return (GetCoachIndex(PC) >= 0);
}

void AKFBB_TeamInfo::RegisterTeamMember(AKFBB_PlayerPawn* P)
{
	if (!P || IsTeamMember(P)) { return; }

	FTeamMember t;
	t.Player = P;
	t.AI = Cast<AKFBB_AIController>(P->GetController());
	MemberList.Add(t);

	P->SetTeamID(TeamID);
}

int32 AKFBB_TeamInfo::GetCoachIndex(AKFBB_CoachPC* C)
{
	return (C ? CoachList.Find(C) : -1);
}

void AKFBB_TeamInfo::SetTeamID(uint8 teamID)
{
	TeamID = teamID;
}

uint8 AKFBB_TeamInfo::GetTeamID() const
{
	return TeamID;
}

void AKFBB_TeamInfo::UnregisterTeamMember(AKFBB_PlayerPawn* P)
{
	int32 i = GetTeamMemberIndex(P);
	if (i >= 0)
	{
		MemberList.RemoveAt(i);
	}
}

bool AKFBB_TeamInfo::IsTeamMember(AKFBB_PlayerPawn* P)
{
	return (GetTeamMemberIndex(P) >= 0);
}

void AKFBB_TeamInfo::Reset()
{
	Super::Reset();

	for (int i = 0; i < MemberList.Num(); i++)
	{
		FTeamMember& TM = MemberList[i];
		if (!TM.Player) { continue; }

		FVector StartLocation = TM.SpawnTile->TileLocation + FVector(0, 0, Field->PlayerSpawnOffsetZ);
		FRotator StartRotation(ForceInit);
		StartRotation.Yaw = Field->GetActorRotation().Yaw + ((TeamID == 0) ? 180.f : 0.f);
		TM.Player->TeleportTo(StartLocation, StartRotation, false, true);
	}

	for (int i = 0; i < CoachList.Num(); i++)
	{
		//do something to reset the coaches
	}
}

int32 AKFBB_TeamInfo::GetTeamMemberIndex(AKFBB_PlayerPawn* P)
{
	if (!P) { return -1; }
	for (int32 i = 0; i < MemberList.Num(); i++)
	{
		if (MemberList[i].Player == P) { return i; }
	}
	return -1;
}

