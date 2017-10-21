// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_CoachPC.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
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
}

void AKFBB_CoachPC::PlayerTouchScreen()
{
	FVector WorldLoc, WorldDir;
	if(DeprojectMousePositionToWorld(WorldLoc, WorldDir))
	{
		auto MyWorld = GetWorld();
		
		FHitResult Hit;
		if (MyWorld->LineTraceSingleByChannel(Hit, WorldLoc, WorldLoc + (WorldDir * 10000.f), ECollisionChannel::ECC_Visibility))
		{
			if (DestinationTile != nullptr)
			{
				ClearTileSelection();
			}

			UKFBB_FieldTile* Tile = Cast<UKFBB_FieldTile>(Hit.GetComponent());
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
					DestinationTile = Tile;
				}
			}
		}
		else
		{
			ClearTileSelection();
		}
	}
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