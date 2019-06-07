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

	UpdateDisplayTileUnderMouse();
	CheckDragPath();

	DrawDebug(DeltaTime);
}

void AKFBB_CoachPC::UpdateDisplayTileUnderMouse()
{
	float scalar = DisplayTileUnderMouse ? MouseUnderTileScalar : 0.f;
	auto MouseTile = GetTileUnderMouse(scalar);
	if (MouseTile)
	{
		DisplayTileUnderMouse = MouseTile;
	}	
}

UKFBB_FieldTile* AKFBB_CoachPC::GetTileUnderMouse(float ReqDistFromCenterScale)
{
	FVector WorldLoc, WorldDir;
	if (DeprojectMousePositionToWorld(WorldLoc, WorldDir))
	{
		MouseWorldLoc = WorldLoc;
		MouseWorldDir = WorldDir;
	}

	auto MyWorld = GetWorld();
	FHitResult Hit;
	if (MyWorld->LineTraceSingleByChannel(Hit, MouseWorldLoc, MouseWorldLoc + (MouseWorldDir * 10000.f), ECollisionChannel::ECC_Visibility))
	{
		auto HitTile = Cast<UKFBB_FieldTile>(Hit.GetComponent());
		if (!HitTile || ReqDistFromCenterScale <= 0.f)
		{
			return HitTile;
		}

		float ReqDistFromCenter = HitTile->GetTileSize() * 0.5f * ReqDistFromCenterScale;
		float DistFromCenter = (Hit.Location - HitTile->TileLocation).Size2D();

//		DrawDebugCylinder(GetWorld(), HitTile->TileLocation, HitTile->TileLocation, ReqDistFromCenter, 32, (DistFromCenter <= ReqDistFromCenter) ? FColor::Green : FColor::Red);
//		DrawDebugLine(GetWorld(), HitTile->TileLocation, Hit.Location, FColor::Blue);

		if (DistFromCenter <= ReqDistFromCenter)
		{
			return HitTile;
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

	BeginDragTouch(Tile);
	if (PlayerOnTile)
	{
		SetSelectedPlayer(PlayerOnTile);
	}
}

void AKFBB_CoachPC::PlayerUntouchScreen()
{
	UKFBB_FieldTile* Tile = GetTileUnderMouse();
	bool bDidDragPath = StartDragTile && Tile != StartDragTile && SelectedTileList.Num() > 0;
	if (bDidDragPath)
	{
		SetSelectedTile(SelectedTileList.Last());
	}
	else if (SelectedPlayer)
	{
		bool bOnSelectedTile = SelectedTile && Tile == SelectedTile;
		bool bOnSelectedPlayer = Tile == SelectedPlayer->CurrentTile;
		if (bOnSelectedTile)
		{
			SetDestinationTile(Tile);
			ConfirmCommand();
			SetSelectedPlayer(nullptr);
		}
		else if (bOnSelectedPlayer && PrevSelectedPlayer == SelectedPlayer)
		{
			SetSelectedPlayer(nullptr);
		}
		else
		{
			ClearTileSelection();
			if (!SelectedTile && !bOnSelectedPlayer)
			{
				SetSelectedTile(Tile);
			}
		}
	}

	EndDragTouch(Tile);
}

void AKFBB_CoachPC::BeginDragTouch(UKFBB_FieldTile* Tile)
{
	StartDragTile = Tile;
	bIsDragging = true;
	bIsDraggingPath = Tile->HasPlayer();
}

void AKFBB_CoachPC::EndDragTouch(UKFBB_FieldTile* Tile)
{
	bIsDragging = false;
	bIsDraggingPath = false;
	StartDragTile = nullptr;
}

void AKFBB_CoachPC::CheckDragPath()
{
	if (!bIsDraggingPath) { return; }

	auto MouseTile = GetTileUnderMouse(MouseUnderTileScalar);
	if (!MouseTile) { return; }

	if (MouseTile == StartDragTile) { return; }

	auto LastDragTile = SelectedTileList.Num() ? SelectedTileList.Last() : nullptr;
	if (MouseTile == LastDragTile) { return; }

	if (LastDragTile && !AKFBB_Field::AreAdjacentTiles(LastDragTile, MouseTile))
	{
		while (MouseTile != LastDragTile)
		{
			FTileDir tileDir = FTileDir::ConvertToTileDir(FVector2D(MouseTile->TileLocation - LastDragTile->TileLocation));
			LastDragTile = LastDragTile->GetAdjacentTile(tileDir);
			if (!LastDragTile)
			{
				//error
				break;
			}

			SelectedTileList.Add(LastDragTile);
		}
	}
	else
	{
		SelectedTileList.Add(MouseTile);
	}
}

void AKFBB_CoachPC::SetSelectedPlayer(AKFBB_PlayerPawn* p)
{
	PrevSelectedPlayer = SelectedPlayer;
	SelectedPlayer = p;
	// Start fresh making tile selection when switching selected players
	ClearTileSelection();
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

	bool bSuccess = SelectedPlayer->CanAcceptCommand();
	if (bSuccess)
	{
		bool bHasPath = SelectedTileList.Num() > 1;
		if (bHasPath)
		{
			bSuccess = AI->SetDestinationTile(SelectedTileList);
		}
		else
		{
			bSuccess = AI->SetDestinationTile(DestinationTile);
		}
	}

	if (bSuccess)
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
	SelectedTileList.Empty();
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

void AKFBB_CoachPC::DrawDebug(float DeltaTime)
{
	if (SelectedPlayer != nullptr && SelectedPlayer->CurrentTile != nullptr)
	{
		SelectedPlayer->CurrentTile->DrawDebugTileOverride(FVector(0, 0, 4), 0.5f, FColor::Green);
	}

	if (SelectedTile != nullptr)
	{
		if (SelectedTile == DisplayTileUnderMouse && bIsDragging)
		{
			DebugFlashSelectedTileTimer -= DeltaTime;
			if (DebugFlashSelectedTileTimer <= 0)
			{
				DebugFlashSelectedTileTimer = 0.5f;
				bDebugHighlightSelectedTile = !bDebugHighlightSelectedTile;
			}
		}
		else
		{
			bDebugHighlightSelectedTile = false;
		}

		float thickness = bDebugHighlightSelectedTile ? 5.f : 0.f;

		SelectedTile->DrawDebugTileOverride(FVector(0, 0, 2), 0.45f, FColor::White, thickness);
		
	}
	for (int i = 0; i < SelectedTileList.Num(); i++)
	{
		SelectedTileList[i]->DrawDebugTileOverride(FVector(0, 0, 2), 0.45f, i == SelectedTileList.Num() - 1 ? FColor::White : FColor::Emerald);
	}
	if (DestinationTile != nullptr)
	{
		DestinationTile->DrawDebugTileOverride(FVector(0, 0, 2), 0.45f, FColor::Purple);
	}

	if (DisplayTileUnderMouse != nullptr)
	{
		DisplayTileUnderMouse->DrawDebugTileOverride(FVector(0, 0, 2), 0.4f, FColor::Yellow);
	}
}