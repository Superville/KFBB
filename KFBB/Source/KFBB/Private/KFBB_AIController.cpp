// Fill out your copyright notice in the Description page of Project Settings.
#include "KFBB_AIController.h"

// Engine Includes
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"

// KFBB Includes
#include "KFBB_PlayerPawn.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "Navigation/GridPathFollowingComponent.h"
#include "Player/KFBBAttributeSet.h"

AKFBB_AIController::AKFBB_AIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGridPathFollowingComponent>(TEXT("PathFollowingComponent")))
{
}

void AKFBB_AIController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	MyPlayerPawn = Cast<AKFBB_PlayerPawn>(InPawn);
}

AKFBB_Field* AKFBB_AIController::GetField() const
{
	if (MyPlayerPawn != nullptr)
	{
		return MyPlayerPawn->Field;
	}
	return nullptr;
}

UKFBB_FieldTile* AKFBB_AIController::GetDestinationTile()
{
	return MyPlayerPawn ? MyPlayerPawn->DestinationTile : nullptr;
}

TArray<UKFBB_FieldTile*> AKFBB_AIController::GetDestinationPath()
{
	if (MyPlayerPawn)
	{
		return MyPlayerPawn->PathToDestTile;
	}
	TArray<UKFBB_FieldTile*> EmptyArray;
	return EmptyArray;
}

void AKFBB_AIController::ClearPathing(TArray<UKFBB_FieldTile*>& out_PathList)
{
	out_PathList.Empty();

	AKFBB_Field* Field = GetField();
	if (Field == nullptr)
		return;

	for (int i = 0; i < Field->Tiles.Num(); i++)
	{
		UKFBB_FieldTile* t = Field->Tiles[i];
		t->bPathVisited = false;
		t->pathHeuristicCost = 0.f;
		t->pathGlobalCost = 0.f;
		t->pathTotalCost = 0.f;
		t->pathPrevTile = nullptr;
	}
}

float AKFBB_AIController::GetPathGlobalCost(UKFBB_FieldTile* curr, UKFBB_FieldTile* next) const
{
	return curr->pathGlobalCost + (curr->TileLocation - next->TileLocation).SizeSquared2D();
}

float AKFBB_AIController::GetPathHeuristicCost(UKFBB_FieldTile* dest, UKFBB_FieldTile* next) const
{
	float cost = (dest->TileLocation - next->TileLocation).SizeSquared2D();
	
	return cost;
}

bool AKFBB_AIController::CanMoveThruTile(UKFBB_FieldTile* tile) const
{
	if (!tile) { return false; }

	bool bSuccess = true;

	auto TilePlayer = tile->GetPlayer();
	if (TilePlayer && TilePlayer != MyPlayerPawn)
	{
		bSuccess = !MyPlayerPawn->IsSameTeam(TilePlayer) || TilePlayer->IsStatusMoving();
	}

	return bSuccess;
}

bool AKFBB_AIController::MarkDestinationTile(UKFBB_FieldTile* DestTile)
{
	ClearDestination();

	auto P = Cast<AKFBB_PlayerPawn>(GetPawn());
	if (!P || !DestTile) { return false; }

	P->SetDestinationTile(DestTile);
	
	if (P->DestinationTile == P->CurrentTile) { return true; }

	int MaxPathLength = P->AttribSet ? P->AttribSet->Stat_Movement.GetCurrentValue() + 1 : 0;
	bool bSuccess = GeneratePathToTile(P->CurrentTile, P->DestinationTile, P->PathToDestTile);
	bSuccess = bSuccess && GetDestinationPath().Num() <= MaxPathLength;

	return bSuccess;
}

bool AKFBB_AIController::MarkDestinationPath(TArray<UKFBB_FieldTile*>& ProvidedPath)
{
	ClearDestination();

	auto P = Cast<AKFBB_PlayerPawn>(GetPawn());
	if (!P || ProvidedPath.Num() <= 0)	{ return false; }

	P->SetDestinationTile(ProvidedPath.Last());
	bool bSuccess = (P->DestinationTile == P->CurrentTile);
	if (!bSuccess)
	{
		int MaxPathLength = P->AttribSet ? P->AttribSet->Stat_Movement.GetCurrentValue() + 1 : 0;
		P->SetPathToDestTile(ProvidedPath);
		bSuccess = bSuccess && GetDestinationPath().Num() <= MaxPathLength;

		//todo verify path provided are adjacent files
	}

	return bSuccess;
}


bool AKFBB_AIController::ConfirmMoveToDestinationTile()
{
	auto BB = GetBlackboardComponent();
	if (!BB) { return false; }

	BB->SetValueAsBool(PathSetName, true);

	bAbortGridMove = false;
	return true;
}

void AKFBB_AIController::ClearDestination()
{
	if (!MyPlayerPawn) { return; }
	MyPlayerPawn->ClearDestinationTile();
	MyPlayerPawn->ClearPathToDestTile();

	bAbortGridMove = true;

	auto BB = GetBlackboardComponent();
	if (BB) 
	{ 
		BB->ClearValue(PathSetName); 
	}
}

bool AKFBB_AIController::AddToPath(UKFBB_FieldTile* StartTile, UKFBB_FieldTile* DestTile, TArray<UKFBB_FieldTile*>& out_PathList)
{
	auto P = Cast<AKFBB_PlayerPawn>(GetPawn());
	if (!P || !StartTile || !DestTile) { return false; }

	if (StartTile == DestTile) { return true; }

	bool bSuccess = false;

	int DestIdx = out_PathList.Find(DestTile);
	if (DestIdx >= 0)
	{
		// If dest tile is already in our path, clip the path
		int RemoveIdx = DestIdx + 1;
		int RemoveCnt = out_PathList.Num() - RemoveIdx;
		out_PathList.RemoveAt(RemoveIdx, RemoveCnt);
	}
	else
	{
		TArray<UKFBB_FieldTile*> PathToAdd;
		int MaxPathLength = P->AttribSet ? P->AttribSet->Stat_Movement.GetCurrentValue() + 1 : 0;
		bSuccess = GeneratePathToTile(StartTile, DestTile, PathToAdd);

		if (bSuccess)
		{
			// If our path has already started to be built
			if (out_PathList.Num() > 0)
			{
				// Strip out the redundant starting tile at the start
				PathToAdd.RemoveAt(0);
			}

			out_PathList.Append(PathToAdd);
			if (out_PathList.Num() > MaxPathLength)
			{
				out_PathList.SetNum(MaxPathLength);
			}
		}
	}

	return bSuccess;
}

bool AKFBB_AIController::GeneratePathToTile(UKFBB_FieldTile* StartTile, UKFBB_FieldTile* DestTile, TArray<UKFBB_FieldTile*>& out_PathList, int MaxPathLength)
{
	AKFBB_Field* Field = GetField();
	if (Field == nullptr)
		return false;

	UKFBB_FieldTile* CurrentTile = StartTile;
	if (CurrentTile == nullptr)
		return false;

	if (DestTile == nullptr)
		return false;

	ClearPathing(out_PathList);

	TArray<UKFBB_FieldTile*> ClosedSet;
	TArray<UKFBB_FieldTile*> OpenSet;
	OpenSet.Add(CurrentTile);
	CurrentTile->bPathVisited = true;

	bool bFoundPath = false;
	while (OpenSet.Num() > 0)
	{
		UKFBB_FieldTile* curr = OpenSet[0];
		OpenSet.RemoveAt(0);

		TArray<UKFBB_FieldTile*> neighborlist;
		for (int dir = 0; dir < Field->TileDirections.Num(); dir++)
		{
			UKFBB_FieldTile* n = Field->GetAdjacentTile(curr, Field->TileDirections[dir]);
			if (n != nullptr)
			{
				neighborlist.Add(n);
			}
		}

		for (int n = 0; n < neighborlist.Num(); n++)
		{
			UKFBB_FieldTile* neighbor = neighborlist[n];
			if (neighbor == DestTile)
			{
				neighbor->pathPrevTile = curr;
				bFoundPath = true;
				break;
			}

			if (!CanMoveThruTile(neighbor))
			{
				continue;
			}

			float globalCost = GetPathGlobalCost(curr, neighbor);
			float heurCost = GetPathHeuristicCost(DestTile, neighbor);
			float totalCost = globalCost + heurCost;

			if (!neighbor->bPathVisited || totalCost < neighbor->pathTotalCost)
			{
				neighbor->bPathVisited = true;
				neighbor->pathPrevTile = curr;
				neighbor->pathGlobalCost = globalCost;
				neighbor->pathHeuristicCost = heurCost;
				neighbor->pathTotalCost = totalCost;

				int insertIdx = 0;
				for (int idx = 0; idx < OpenSet.Num(); idx++)
				{
					UKFBB_FieldTile* ost = OpenSet[idx];
					if (neighbor->pathTotalCost < ost->pathTotalCost)
					{
						break;
					}
					insertIdx++;
				}
				OpenSet.Insert(neighbor, insertIdx);
			}
		}

		if (bFoundPath)
		{
			break;
		}
	}
	
	// reconstruct path
	if (bFoundPath)
	{
		UKFBB_FieldTile* buildTile = DestTile;
		while (buildTile != nullptr)
		{
			out_PathList.Insert(buildTile, 0);
			buildTile = buildTile->pathPrevTile;
		}

		if (MaxPathLength > 0 && out_PathList.Num() > (MaxPathLength + 1))
		{
			int removeIdx = (MaxPathLength + 1);
			int removeCnt = out_PathList.Num() - removeIdx;
			out_PathList.RemoveAt(removeIdx, removeCnt);
		}

		return true;
	}
	return false;
}

uint8 AKFBB_AIController::GetTeamID() const
{
	return (MyPlayerPawn ? MyPlayerPawn->GetTeamID() : 255);
}

