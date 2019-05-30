// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_CoachPC.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"

#include "KFBB_PlayerPawn.h"
#include "KFBB_AIController.h"

#include "AIController.h"
#include "KFBB_Ball.h"
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
		SelectedTile->DrawDebugTileOverride(FVector(0, 0, 2), 0.45f, FColor::White);
	}
	if (DestinationTile != nullptr)
	{
		DestinationTile->DrawDebugTileOverride(FVector(0, 0, 2), 0.45f, FColor::Purple);
	}
	if (SelectedPlayer != nullptr && SelectedPlayer->CurrentTile != nullptr)
	{
		SelectedPlayer->CurrentTile->DrawDebugTileOverride(FVector(0, 0, 4), 0.5f, FColor::Green);
	}

	if (SelectedTile != nullptr && 
		DestinationTile == nullptr && 
		MouseTile != nullptr && 
		MouseTile != SelectedTile)
	{
		MouseTile->DrawDebugTileOverride(FVector(0, 0, 2), 0.45f, FColor::Yellow);
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

AKFBB_PlayerPawn* AKFBB_CoachPC::GetSelectedPlayer() const
{
	if (SelectedTile != nullptr)
	{
		return SelectedTile->GetPlayer();
	}
	return nullptr;
}

void AKFBB_CoachPC::PlayerTouchScreen()
{
	UKFBB_FieldTile* Tile = GetTileUnderMouse();
	AKFBB_PlayerPawn* PlayerOnTile = Tile ? Tile->GetPlayer() : nullptr;

	if (PlayerOnTile && PlayerOnTile != SelectedPlayer)
	{
		SetSelectedPlayer(PlayerOnTile);
	}
	else if (SelectedPlayer && !PlayerOnTile)
	{
		if (!SelectedTile || Tile != SelectedTile)
		{
			SetSelectedTile(Tile);
		}
		else if (Tile && Tile == SelectedTile)
		{
			SetDestinationTile(Tile);
			ConfirmCommand();
		}
		else
		{
			ClearTileSelection();
		}
	}

/*
	// If clicked on the same dest tile
	if (DestinationTile != nullptr && Tile == DestinationTile)
	{
		ConfirmCommand();
	}
	else if (DestinationTile != nullptr || Tile == nullptr)
	{
		ClearTileSelection();
	}
	else if (Tile != nullptr)
	{
		//DrawDebugTouchedTile(Tile);

		if (SelectedTile == nullptr || SelectedTile->HasPlayer() == false)
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
	}*/
}

void AKFBB_CoachPC::SetSelectedPlayer(AKFBB_PlayerPawn* p)
{
	SelectedPlayer = p;
}

void AKFBB_CoachPC::SetSelectedTile(UKFBB_FieldTile* t)
{
	SelectedTile = t;
}

void AKFBB_CoachPC::SetDestinationTile(UKFBB_FieldTile* t)
{
	DestinationTile = t;
}

void AKFBB_CoachPC::ConfirmCommand()
{
	if (!DestinationTile || !SelectedPlayer) { return; }
	auto AI = Cast<AKFBB_AIController>(SelectedPlayer->Controller);
	if (!AI) { return; }

	if (SelectedPlayer->CanAcceptCommand() &&
		AI->SetDestinationTile(DestinationTile))
	{
		SelectedPlayer->SetStatus(EKFBB_PlayerState::Moving);
	}
	else
	{
		SelectedPlayer->NotifyCommandFailed();
	}
}

void AKFBB_CoachPC::SpawnPlayerOnTile()
{
	int x = 0, y = 0;
	UKFBB_FieldTile* Tile = GetTileUnderMouse();
	if (Tile != nullptr)
	{
		x = Tile->TileX;
		y = Tile->TileY;
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

void AKFBB_CoachPC::SpawnBallOnTile()
{
	int x = 0, y = 0;
	UKFBB_FieldTile* Tile = GetTileUnderMouse();
	if (Tile != nullptr)
	{
		x = Tile->TileX;
		y = Tile->TileY;
	}

	SpawnBallOnTile(x, y);
}

void AKFBB_CoachPC::SpawnBallOnTile(int x, int y)
{
	if (BallClass == nullptr)
		return;

	auto World = GetWorld();

	FTransform SpawnTrans;
	SpawnTrans.SetLocation(Field->GetFieldTileLocation(x, y) + FVector(0, 0, PlayerSpawnOffsetZ));
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;

	AKFBB_Ball* P = World->SpawnActor<AKFBB_Ball>(BallClass, SpawnTrans, SpawnParams);
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