// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_TeamInfo.h"

// KFBB Includes
#include "KFBB_PlayerPawn.h"
#include "KFBB_AIController.h"
#include "KFBB_CoachPC.h"

void AKFBB_TeamInfo::RegisterCoach(AKFBB_CoachPC* PC)
{
	if (!PC || GetCoachIndex(PC) >= 0) { return; }
	CoachList.Add(PC);
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

int32 AKFBB_TeamInfo::GetTeamMemberIndex(AKFBB_PlayerPawn* P)
{
	if (!P) { return -1; }
	for (int32 i = 0; i < MemberList.Num(); i++)
	{
		if (MemberList[i].Player == P) { return i; }
	}
	return -1;
}

