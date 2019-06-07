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

void AKFBB_AIController::ClearPathing()
{
	PathToDestTile.Empty();

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
	if (!CanMoveThruTile(next))
	{
		cost += 10000;
	}
	return cost;
}

bool AKFBB_AIController::CanMoveThruTile(UKFBB_FieldTile* tile) const
{
	if (tile != nullptr)
	{
		if (tile->HasPlayer() && !MyPlayerPawn->bRunThroughPlayers)
		{
			//todo should check for enemy player
			return false;
		}
	}

	return true;
}

bool AKFBB_AIController::SetDestinationTile(UKFBB_FieldTile* DestTile)
{
	ClearDestinationTile();
	if (!DestTile)
	{
		return true;
	}

	auto BB = GetBlackboardComponent();
	if (!BB) { return false; }
	auto P = Cast<AKFBB_PlayerPawn>(GetPawn());
	if (!P) { return false; }

	DestinationTile = DestTile;
	
	bool bSuccess = (DestinationTile == P->CurrentTile);
	if (!bSuccess)
	{
		bSuccess = GeneratePathToTile(DestinationTile);
	}

	if (bSuccess)
	{
		BB->SetValueAsBool(PathSetName, true);
	}

	bAbortGridMove = false;
	return bSuccess;
}

bool AKFBB_AIController::SetDestinationTile(TArray<UKFBB_FieldTile*>& ProvidedPath)
{
	ClearDestinationTile();
	if (ProvidedPath.Num() <= 0)
	{
		return true;
	}

	auto BB = GetBlackboardComponent();
	if (!BB) { return false; }
	auto P = Cast<AKFBB_PlayerPawn>(GetPawn());
	if (!P) { return false; }

	DestinationTile = ProvidedPath.Last();

	bool bSuccess = (DestinationTile == P->CurrentTile);
	if (!bSuccess)
	{
		PathToDestTile = ProvidedPath;

		//todo verify path provided are adjacent files

		bSuccess = true;
	}

	if (bSuccess)
	{
		BB->SetValueAsBool(PathSetName, true);
	}

	bAbortGridMove = false;
	return bSuccess;
}

void AKFBB_AIController::ClearDestinationTile()
{
	DestinationTile = nullptr;
	PathToDestTile.Empty();
	bAbortGridMove = true;

	auto BB = GetBlackboardComponent();
	if (BB) 
	{ 
		BB->ClearValue(PathSetName); 
	}
}

bool AKFBB_AIController::GeneratePathToTile(UKFBB_FieldTile* DestTile)
{
	AKFBB_Field* Field = GetField();
	if (Field == nullptr)
		return false;

	UKFBB_FieldTile* CurrentTile = MyPlayerPawn->CurrentTile;
	if (CurrentTile == nullptr)
		return false;

	if (DestTile == nullptr)
		return false;

	ClearPathing();

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
			PathToDestTile.Insert(buildTile, 0);
			buildTile = buildTile->pathPrevTile;
		}		
		return true;
	}
	return false;
}

void AKFBB_AIController::DrawDebugPath() const
{
	FVector offset(0, 0, 2);
	UKFBB_FieldTile* LastTile = nullptr;
	for (int i = 0; i < PathToDestTile.Num(); i++)
	{
		auto CurrentTile = PathToDestTile[i];
		CurrentTile->DrawDebugTileOverride(offset, 0.25f, FColor::Emerald);
		if (LastTile != nullptr)
		{
			DrawDebugLine(GetWorld(), LastTile->TileLocation + offset, CurrentTile->TileLocation + offset, FColor::Emerald);
		}
		else
		{
			FVector PawnLoc = GetPawn()->GetActorLocation();
			PawnLoc.Z = CurrentTile->TileLocation.Z;
			DrawDebugLine(GetWorld(), PawnLoc + offset, CurrentTile->TileLocation + offset, FColor::Emerald);
		}
		LastTile = PathToDestTile[i];
	}
}