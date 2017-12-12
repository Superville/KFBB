// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_CoachPC.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "AIController.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

void AKFBB_CoachPC::BeginPlay()
{
	Super::BeginPlay();

	AKFBB_Field::AssignFieldActor(this, Field);
}

void AKFBB_CoachPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto MouseTile = GetTileUnderMouse();

	if (SelectedTile != nullptr)
	{
		FColor c = FColor::White;
		DrawDebugBox(GetWorld(), SelectedTile->TileLocation + FVector(0, 0, 2), FVector(Field->TileSize, Field->TileSize, 0) * 0.45f, c, false);
	}
	if (DestinationTile != nullptr)
	{
		FColor c = FColor::Purple;
		DrawDebugBox(GetWorld(), DestinationTile->TileLocation + FVector(0, 0, 2), FVector(Field->TileSize, Field->TileSize, 0) * 0.45f, c, false);
	}

	if (SelectedTile != nullptr && 
		DestinationTile == nullptr && 
		MouseTile != nullptr && 
		MouseTile != SelectedTile)
	{
		FColor c = FColor::Yellow;
		DrawDebugBox(GetWorld(), MouseTile->TileLocation + FVector(0, 0, 2), FVector(Field->TileSize, Field->TileSize, 0) * 0.45f, c, false);
	}
}

UKFBB_FieldTile* AKFBB_CoachPC::GetTileUnderMouse()
{
	FVector WorldLoc, WorldDir;
	if (DeprojectMousePositionToWorld(WorldLoc, WorldDir))
	{
		auto MyWorld = GetWorld();

		FHitResult Hit;
		if (MyWorld->LineTraceSingleByChannel(Hit, WorldLoc, WorldLoc + (WorldDir * 10000.f), ECollisionChannel::ECC_Visibility))
		{
			return Cast<UKFBB_FieldTile>(Hit.GetComponent());
		}
	}
	return nullptr;
}

void AKFBB_CoachPC::PlayerTouchScreen()
{
	UKFBB_FieldTile* Tile = GetTileUnderMouse();

	if (DestinationTile != nullptr || Tile == nullptr)
	{
		ClearTileSelection();
	}

	if (Tile != nullptr)
	{
		//DrawDebugTouchedTile(Tile);

		if (SelectedTile == nullptr)
		{
			SelectedTile = Tile;
		}
		else if (SelectedTile == Tile)
		{
			ClearTileSelection();
		}
		else
		{
			SetDestinationTile(Tile);
		}
	}
}

void AKFBB_CoachPC::SetDestinationTile(UKFBB_FieldTile* t)
{
	DestinationTile = t;
	if (DestinationTile != nullptr && SelectedTile != nullptr && SelectedTile->HasPlayer())
	{
		TryMovePawnToDestination();
	}
}

bool AKFBB_CoachPC::TryMovePawnToDestination()
{
	AKFBB_PlayerPawn* p = SelectedTile->GetPlayer();
	if (p != nullptr )
	{
		if (p->CanAcceptCommand() && p->Controller != nullptr)
		{
			AAIController* ai = Cast<AAIController>(p->Controller);
			if (ai != nullptr)
			{
				FVector Dest = DestinationTile->TileLocation;
				Dest.Z += PlayerSpawnOffsetZ;
				auto result = ai->MoveToLocation(Dest, 0.f, false, false, false, false);
				
				if (result == EPathFollowingRequestResult::RequestSuccessful)
				{
					return true;
				}
			}
		}

		p->NotifyCommandFailed();
	}

	return false;
}

void AKFBB_CoachPC::SpawnPlayerOnTile()
{
	int x = 0, y = 0;
	if (SelectedTile != nullptr)
	{
		x = SelectedTile->x;
		y = SelectedTile->y;
	}
	
	SpawnPlayerOnTile(x, y);
}

void AKFBB_CoachPC::SpawnPlayerOnTile(int x, int y)
{
	if (PlayerClass == nullptr)
		return;

	auto World = GetWorld();

	FTransform SpawnTrans;
	SpawnTrans.SetLocation(Field->GetFieldTileLocation(x, y) + FVector(0, 0, PlayerSpawnOffsetZ));
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;

	AKFBB_PlayerPawn* P = World->SpawnActor<AKFBB_PlayerPawn>(PlayerClass, SpawnTrans, SpawnParams);
	P->Coach = this;
}

void AKFBB_CoachPC::ClearTileSelection()
{
	SelectedTile = nullptr;
	DestinationTile = nullptr;
}

void AKFBB_CoachPC::DrawDebugTouchedTile(UKFBB_FieldTile* t)
{
	auto MyWorld = GetWorld();

	FColor color = FColor::Green;
	if (t->bEndZone) color = FColor::Red;
	else if (t->bWideOut) color = FColor::Orange;
	else if (t->bScrimmageLine) color = FColor::Cyan;

	DrawDebugBox(MyWorld, t->TileLocation + FVector(0, 0, 2), FVector(Field->TileSize, Field->TileSize, 0) * 0.5f, color, false, 1.f);
}