// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_CoachPC.h"

// Engine Includes
#include "Engine.h"
#include "AIController.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"

// KFBB Includes
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "KFBB_PlayerPawn.h"
#include "KFBB_AIController.h"
#include "Player/KFBBAttributeSet.h"
#include "KFBB_Ball.h"
#include "KFBBGameModeBase.h"

void AKFBB_CoachPC::BeginPlay()
{
	Super::BeginPlay();

	AKFBB_Field::AssignFieldActor(this, Field);

	SetTeamID(0);
}

void AKFBB_CoachPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateDisplayTileUnderMouse();
	CheckDragPath();

	DrawPotentialMoveList();
	DrawSelectedPlayer();

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

	if (!PlayerOnTile || PlayerOnTile->CanAcceptCommand())
	{
		BeginDragTouch(Tile);
	}
	if (PlayerOnTile)
	{
		SetSelectedPlayer(PlayerOnTile);
	}
}

void AKFBB_CoachPC::PlayerUntouchScreen()
{
	UKFBB_FieldTile* Tile = GetTileUnderMouse();
	bool bDidDragPath = StartDragTile && Tile != StartDragTile && SelectedTileList.Num() > 0;
	
	if (!bDidDragPath && SelectedPlayer)
	{
		bool bOnSelectedTile = SelectedTile && Tile == SelectedTile;
		bool bOnSelectedPlayer = Tile == SelectedPlayer->CurrentTile;
		if (bOnSelectedTile && !bOnSelectedPlayer)
		{
			SetDestinationTile(Tile);
			ConfirmCommand();
			ClearSelectedPlayer();
		}
		else if (bOnSelectedPlayer && PrevSelectedPlayer == SelectedPlayer)
		{
			ClearSelectedPlayer();
		}
		else if(!bOnSelectedPlayer && SelectedPlayer && SelectedPlayer->CanAcceptCommand())
		{
			if (PotentialMoveList.Find(Tile) == INDEX_NONE)
			{
				ClearTileSelection();
				if (!SelectedTile && !bOnSelectedPlayer)
				{
					SetSelectedTile(Tile);
				}
			}
			else
			{
				int TileIdx = SelectedTileList.Find(Tile);
				if (TileIdx == INDEX_NONE)
				{
					AddToPath(Tile);
				}
				else
				{
					SelectedTileList.RemoveAt(TileIdx + 1, SelectedTileList.Num() - (TileIdx + 1));
					SetSelectedTile(SelectedTileList);
				}
			}
		}
	}

	EndDragTouch(Tile);
}

void AKFBB_CoachPC::BeginDragTouch(UKFBB_FieldTile* Tile)
{
	StartDragTile = Tile;
	bIsDragging = true;
	bIsDraggingPath = Tile ? Tile->HasPlayer() : false;
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
	if (MouseTile == StartDragTile) { return; }

	AddToPath(MouseTile);
}

void AKFBB_CoachPC::AddToPath(UKFBB_FieldTile* Tile)
{
	if (!Tile || !SelectedPlayer || !SelectedAI) { return; }

	if (!SelectedAI->CanMoveThruTile(Tile)) { return; }

	bool bSuccess = false;

	int TileIdx = SelectedTileList.Find(Tile);
	if (TileIdx == INDEX_NONE)
	{
		auto LastDragTile = SelectedTileList.Num() ? SelectedTileList.Last() : SelectedPlayer->CurrentTile;
		if (Tile == LastDragTile) { return; }

		bSuccess = SelectedAI->AddToPath(LastDragTile, Tile, SelectedTileList);
	}
	else
	{
		SelectedTileList.RemoveAt(TileIdx + 1, SelectedTileList.Num() - (TileIdx + 1));
		bSuccess = true;
	}

	if (bSuccess)
	{
		SetSelectedTile(SelectedTileList);
	}
}

void AKFBB_CoachPC::SetSelectedPlayer(AKFBB_PlayerPawn* p)
{
	if (!p) { return; }

	if(p->CanBeSelected(this))
	{
		PrevSelectedPlayer = SelectedPlayer;
		SelectedPlayer = p;
		SelectedAI = SelectedPlayer ? Cast<AKFBB_AIController>(SelectedPlayer->GetController()) : nullptr;
	
		// Start fresh making tile selection when switching selected players
		ClearTileSelection();
	}
}

void AKFBB_CoachPC::ClearSelectedPlayer()
{
	PrevSelectedPlayer = SelectedPlayer;
	SelectedPlayer = nullptr;
	SelectedAI = nullptr;
}

void AKFBB_CoachPC::UpdatePotentialMoveList()
{
	PotentialMoveList.Empty();
	if (SelectedPlayer && SelectedPlayer->CanAcceptCommand() && Field)
	{
		int AvailRange = FMath::FloorToInt(SelectedPlayer->AttribSet->Stat_Movement.GetCurrentValue()) - SelectedTileList.Num();
		if (SelectedTileList.Num() > 0) { AvailRange++; }
		auto StartTile = SelectedTileList.Num() > 0 ? SelectedTileList.Last() : SelectedPlayer->CurrentTile;
		PotentialMoveList = Field->GetListOfTilesInRange(StartTile, AvailRange);
	}
}

void AKFBB_CoachPC::SetSelectedTile(UKFBB_FieldTile* t)
{
	SelectedTile = t;

	if (!SelectedAI || !SelectedTile) { return; }

	bool bSuccess = SelectedAI->SetDestination(SelectedTile);
	if (!bSuccess)
	{
		SelectedAI->ClearDestination();
		SelectedTile = nullptr;
	}
	SelectedTileList = SelectedAI->PathToDestTile;
	UpdatePotentialMoveList();
}

void AKFBB_CoachPC::SetSelectedTile(TArray<UKFBB_FieldTile*>& ProvidedPath)
{
	SelectedTile = ProvidedPath.Num() > 0 ? ProvidedPath.Last() : nullptr;

	if (!SelectedAI || !SelectedTile) { return; }

	bool bSuccess = SelectedAI->SetDestination(ProvidedPath);
	UpdatePotentialMoveList();
}

void AKFBB_CoachPC::SetDestinationTile(UKFBB_FieldTile* t)
{
	DestinationTile = t;
}

void AKFBB_CoachPC::ClearTileSelection(bool bClearAI)
{
	SelectedTile = nullptr;
	DestinationTile = nullptr;
	SelectedTileList.Empty();

	if (SelectedAI && bClearAI)
	{
		SelectedAI->ClearDestination();
	}
	UpdatePotentialMoveList();
}

void AKFBB_CoachPC::ConfirmCommand()
{
	if (!DestinationTile || !SelectedPlayer) { return; }
	if (!SelectedAI) { return; }

	if (SelectedPlayer->CanAcceptCommand())
	{
		SelectedAI->ConfirmMoveToDestinationTile();
		SelectedPlayer->SetStatus(EKFBB_PlayerState::Moving);
		ClearTileSelection(false);
	}
	else
	{
		SelectedPlayer->NotifyCommandFailed();
		ClearTileSelection();
	}
}

void AKFBB_CoachPC::SpawnPlayerOnTile(uint8 teamID)
{
	int x = 0, y = 0;
	UKFBB_FieldTile* Tile = GetTileUnderMouse();
	if (Tile != nullptr)
	{
		x = Tile->TileX;
		y = Tile->TileY;
	}
	
	SpawnPlayerOnTile(x, y, teamID);
}

void AKFBB_CoachPC::SpawnPlayerOnTile(int x, int y, uint8 teamID)
{
	if (PlayerClass == nullptr)
		return;

	auto World = GetWorld();

	FTransform SpawnTrans;
	SpawnTrans.SetLocation(Field->GetFieldTileLocation(x, y) + FVector(0, 0, PlayerSpawnOffsetZ));
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;

	AKFBB_PlayerPawn* P = World->SpawnActor<AKFBB_PlayerPawn>(PlayerClass, SpawnTrans, SpawnParams);
	if (P)
	{
		auto KFGM = Cast<AKFBBGameModeBase>(GetWorld()->GetAuthGameMode());
		if (KFGM)
		{
			KFGM->RegisterTeamMember(P, teamID);
		}
	}
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

void AKFBB_CoachPC::DrawPotentialMoveList()
{
	if (SelectedPlayer)
	{
		TArray<FVector> EdgeList = Field->GetExternalEdgeVerts(PotentialMoveList);
		for (int i = 0; i < EdgeList.Num(); i += 2)
		{
			FVector a = EdgeList[i];
			FVector b = EdgeList[i + 1];

			FVector offset(0, 0, 2);
			DrawDebugLine(GetWorld(), a + offset, b + offset, FColor::White, false, -1, 0, 3.f);
		}
	}
}

void AKFBB_CoachPC::DrawSelectedPlayer()
{
	if (SelectedPlayer != nullptr && SelectedPlayer->CurrentTile != nullptr)
	{
		FVector DrawLocation = SelectedPlayer->GetActorLocation() + FVector(0, 0, -48);
		float Radius = SelectedPlayer->CurrentTile->GetTileSize() * 0.5f;
		DrawDebugCylinder(GetWorld(), DrawLocation, DrawLocation, Radius, 32, FColor::White, false, -1, 0, 3.f);
	}
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

uint8 AKFBB_CoachPC::GetTeamID()
{
	return TeamID;
}

void AKFBB_CoachPC::SetTeamID(uint8 teamID)
{
	auto KBGM = Cast<AKFBBGameModeBase>(GetWorld()->GetAuthGameMode());
	if (KBGM)
	{
		KBGM->RegisterCoach(this, teamID);
	}
}

void AKFBB_CoachPC::DrawDebug(float DeltaTime)
{
	if (SelectedTile != nullptr)
	{
		float thickness = 0.f;
		if (SelectedTile == DisplayTileUnderMouse && bIsDragging)
		{
			DebugFlashSelectedTileTimer -= DeltaTime;
			if (DebugFlashSelectedTileTimer <= 0)
			{
				DebugFlashSelectedTileTimer = 0.5f;
				bDebugHighlightSelectedTile = !bDebugHighlightSelectedTile;
			}
			thickness = bDebugHighlightSelectedTile ? 5.f : 0.f;
		}
		else
		{
			DebugFlashSelectedTileTimer = 0.5f;
			bDebugHighlightSelectedTile = true;
		}

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