// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_CoachPC.h"

// Engine Includes
#include "Engine.h"
#include "AIController.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"
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

void AKFBB_CoachPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKFBB_CoachPC, bReadyToStart);

	DOREPLIFETIME(AKFBB_CoachPC, RepSelectedTileList);
	DOREPLIFETIME(AKFBB_CoachPC, RepSelectedTile);
	DOREPLIFETIME(AKFBB_CoachPC, RepDestinationTile);

	DOREPLIFETIME(AKFBB_CoachPC, SelectedPlayer);
	DOREPLIFETIME(AKFBB_CoachPC, PrevSelectedPlayer);
}

void AKFBB_CoachPC::OnRep_SelectedTileList()
{
	if (!Field) { return; }
	Field->ConvertArrayIndexToTile(RepSelectedTileList, SelectedTileList);
}

void AKFBB_CoachPC::OnRep_SelectedTile()
{
	if (!Field) { return; }
	SelectedTile = Field->GetTileByIndex(RepSelectedTile);
}

void AKFBB_CoachPC::OnRep_DestinationTile()
{
	if (!Field) { return; }
	DestinationTile = Field->GetTileByIndex(RepDestinationTile);
}

void AKFBB_CoachPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateDisplayTileUnderMouse();
	CheckDragPath();
	//test
	UpdatePotentialMoveList();


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
	UKFBB_FieldTile* MouseTile = GetTileUnderMouse();
	AKFBB_PlayerPawn* PlayerOnTile = MouseTile ? MouseTile->GetPlayer() : nullptr;

	bool bBeginDrag = false;
	if (!PlayerOnTile || PlayerOnTile->CanAcceptCommand())
	{
		bBeginDrag = BeginDragTouch(MouseTile);
	}

	ServerTouchScreen(MouseTile ? MouseTile->TileIdx : -1, bBeginDrag);
}

void AKFBB_CoachPC::ServerTouchScreen_Implementation(int TileIdx, bool bBeginDrag)
{
	if (!Field) { return; }

	UKFBB_FieldTile* MouseTile = Field->GetTileByIndex(TileIdx);
	AKFBB_PlayerPawn* PlayerOnTile = MouseTile ? MouseTile->GetPlayer() : nullptr;

	if (bBeginDrag)
	{
		SetDragInfo(MouseTile);
	}

	if (PlayerOnTile)
	{
		SetSelectedPlayer(PlayerOnTile);
	}
}
bool AKFBB_CoachPC::ServerTouchScreen_Validate(int TileIdx, bool bBeginDrag) { return true; }

void AKFBB_CoachPC::PlayerUntouchScreen()
{
	UKFBB_FieldTile* MouseTile = GetTileUnderMouse();
	ServerPlayerUntouchScreen(MouseTile ? MouseTile->TileIdx : -1);
	EndDragTouch(MouseTile);
}

void AKFBB_CoachPC::ServerPlayerUntouchScreen_Implementation(int TileIdx)
{
	if (!Field) { return; }

	UKFBB_FieldTile* MouseTile = Field->GetTileByIndex(TileIdx);
	bool bDidDragPath = StartDragTile && MouseTile != StartDragTile && SelectedTileList.Num() > 0;

	if (!bDidDragPath && SelectedPlayer)
	{
		bool bOnSelectedTile = SelectedTile && MouseTile == SelectedTile;
		bool bOnSelectedPlayer = MouseTile == SelectedPlayer->CurrentTile;
		if (bOnSelectedTile && !bOnSelectedPlayer)
		{
			SetDestinationTile(MouseTile);
			ConfirmCommand();
			ClearSelectedPlayer();
		}
		else if (bOnSelectedPlayer && PrevSelectedPlayer == SelectedPlayer)
		{
			ClearSelectedPlayer();
		}
		else if (!bOnSelectedPlayer && SelectedPlayer && SelectedPlayer->CanAcceptCommand())
		{
			if (PotentialMoveList.Find(MouseTile) == INDEX_NONE)
			{
				ClearTileSelection();
				if (!SelectedTile && !bOnSelectedPlayer)
				{
					MarkSelectedTile(MouseTile);
				}
			}
			else
			{
				int SelectedTileIdx = SelectedTileList.Find(MouseTile);
				if (SelectedTileIdx == INDEX_NONE)
				{
					AddToPath(MouseTile);
				}
				else
				{
					SelectedTileList.RemoveAt(SelectedTileIdx + 1, SelectedTileList.Num() - (SelectedTileIdx + 1));
					MarkSelectedTileList(SelectedTileList);
				}
			}
		}
	}

	EndDragTouch(MouseTile);
}

bool AKFBB_CoachPC::ServerPlayerUntouchScreen_Validate(int TileIdx) { return true; }

bool AKFBB_CoachPC::BeginDragTouch(UKFBB_FieldTile* Tile)
{
	if (bIsDragging) { return false; }
	SetDragInfo(Tile);	
	return true;
}

void AKFBB_CoachPC::EndDragTouch(UKFBB_FieldTile* Tile)
{
	ClearDragInfo();
}

void AKFBB_CoachPC::SetDragInfo(UKFBB_FieldTile* Tile)
{
	StartDragTile = Tile;
	bIsDragging = true;
	bIsDraggingPath = Tile ? Tile->HasPlayer() : false;
}

void AKFBB_CoachPC::ClearDragInfo()
{
	bIsDragging = false;
	bIsDraggingPath = false;
	StartDragTile = nullptr;
}

void AKFBB_CoachPC::CheckDragPath()
{
	if (!bIsDraggingPath) { return; }

	auto MouseTile = GetTileUnderMouse(MouseUnderTileScalar);
	ServerUpdateDragPath(MouseTile ? MouseTile->TileIdx : -1);
}

void AKFBB_CoachPC::ServerUpdateDragPath_Implementation(int TileIdx)
{
	if (!Field) { return; }
	
	UKFBB_FieldTile* MouseTile = Field->GetTileByIndex(TileIdx);
	if (MouseTile == StartDragTile) { return; }

	AddToPath(MouseTile);
}
bool AKFBB_CoachPC::ServerUpdateDragPath_Validate(int TileIdx) { return true; }

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
		MarkSelectedTileList(SelectedTileList);
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

void AKFBB_CoachPC::MarkSelectedTile(UKFBB_FieldTile* Tile)
{
	if (!Tile)
	{
		ClearSelectedTile();
		return;
	}

	SetSelectedTile(Tile);
	if (SelectedAI)
	{
		bool bSuccess = SelectedAI->MarkDestinationTile(SelectedTile);
		if (!bSuccess)
		{
			SelectedAI->ClearDestination();
			SelectedTile = nullptr;
		}
		SelectedTileList = SelectedAI->GetDestinationPath();
	}
	
//test	UpdatePotentialMoveList();
}

void AKFBB_CoachPC::SetSelectedTile(UKFBB_FieldTile* Tile)
{
	if (!Tile) 
	{ 
		ClearSelectedTile();
		return; 
	}
	SelectedTile = Tile;
	RepSelectedTile = Tile->TileIdx;
}

void AKFBB_CoachPC::ClearSelectedTile()
{
	SelectedTile = nullptr;
	RepSelectedTile = -1;
}

void AKFBB_CoachPC::MarkSelectedTileList(TArray<UKFBB_FieldTile*>& ProvidedPath)
{
	if (!Field) { return; }

	SetSelectedTile(ProvidedPath.Num() > 0 ? ProvidedPath.Last() : nullptr);
	SetSelectedTileList(ProvidedPath);
	if (SelectedAI)
	{
		bool bSuccess = SelectedAI->MarkDestinationPath(ProvidedPath);
//test	UpdatePotentialMoveList();
	}
}

void AKFBB_CoachPC::SetSelectedTileList(TArray<UKFBB_FieldTile*>& TileList)
{
	if (!Field) return;
	SelectedTileList = TileList;
	Field->ConvertArrayTileToIndex(TileList, RepSelectedTileList);
}

void AKFBB_CoachPC::ClearSelectedTileList()
{
	SelectedTileList.Empty();
	RepSelectedTileList.Empty();
}

void AKFBB_CoachPC::SetDestinationTile(UKFBB_FieldTile* t)
{
	DestinationTile = t;
	RepDestinationTile = DestinationTile ? DestinationTile->TileIdx : -1;
}

void AKFBB_CoachPC::ClearDestinationTile()
{
	DestinationTile = nullptr;
	RepDestinationTile = -1;
}

void AKFBB_CoachPC::BP_ClearTileSelection(bool bClearAI)
{
	ServerClearTileSelection(bClearAI);
}

void AKFBB_CoachPC::ServerClearTileSelection_Implementation(bool bClearAI)
{
	ClearTileSelection(bClearAI);
}
bool AKFBB_CoachPC::ServerClearTileSelection_Validate(bool bClearAI) { return true; }

void AKFBB_CoachPC::ClearTileSelection(bool bClearAI)
{
	ClearDestinationTile();
	ClearSelectedTile();
	ClearSelectedTileList();

	if (SelectedAI && bClearAI)
	{
		SelectedAI->ClearDestination();
	}
//test	UpdatePotentialMoveList();
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

void AKFBB_CoachPC::SpawnPlayerOnTileUnderMouse(uint8 teamID)
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

void AKFBB_CoachPC::SpawnPlayerOnTile_Implementation(int x, int y, uint8 teamID)
{
	auto World = GetWorld();
	auto KFGM = Cast<AKFBBGameModeBase>(World->GetAuthGameMode());
	if (!Field || !KFGM) { return; }

	auto PlayerClass = KFGM->GetDefaultPlayerClass();
	if (!PlayerClass) { return; }

	FTransform SpawnTrans;
	SpawnTrans.SetLocation(Field->GetFieldTileLocation(x, y) + FVector(0, 0, Field->PlayerSpawnOffsetZ));
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;

	AKFBB_PlayerPawn* P = World->SpawnActor<AKFBB_PlayerPawn>(PlayerClass, SpawnTrans, SpawnParams);
	if (P)
	{
		KFGM->RegisterTeamMember(P, teamID);
	}
}
bool AKFBB_CoachPC::SpawnPlayerOnTile_Validate(int x, int y, uint8 teamID) { return true; }

void AKFBB_CoachPC::SpawnBallOnTileUnderMouse()
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

void AKFBB_CoachPC::SpawnBallOnTile_Implementation(int x, int y)
{
	auto World = GetWorld();
	auto KFGM = Cast<AKFBBGameModeBase>(World->GetAuthGameMode());
	if (!Field || !KFGM) { return; }

	auto BallClass = KFGM->GetDefaultBallClass();
	if (!BallClass) { return; }
	
	FTransform SpawnTrans;
	SpawnTrans.SetLocation(Field->GetFieldTileLocation(x, y) + FVector(0, 0, Field->PlayerSpawnOffsetZ));
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;

	AKFBB_Ball* P = World->SpawnActor<AKFBB_Ball>(BallClass, SpawnTrans, SpawnParams);
}
bool AKFBB_CoachPC::SpawnBallOnTile_Validate(int x, int y) { return true; }

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
	if (t->bEndZone_Team0 || t->bEndZone_Team1) color = FColor::Red;
	else if (t->bWideOut) color = FColor::Orange;
	else if (t->bScrimmageLine) color = FColor::Cyan;

	DrawDebugBox(MyWorld, t->TileLocation + FVector(0, 0, 2), FVector(Field->TileSize, Field->TileSize, 0) * 0.5f, color, false, 1.f);
}

bool AKFBB_CoachPC::IsReadyToStart()
{
	//test
	return true;
	return bReadyToStart;
}

void AKFBB_CoachPC::ToggleReadyToStart_Implementation()
{
	bReadyToStart = !bReadyToStart;
}

bool AKFBB_CoachPC::ToggleReadyToStart_Validate() { return true; }

uint8 AKFBB_CoachPC::GetTeamID() const
{
	return TeamID;
}

void AKFBB_CoachPC::SetTeamID(uint8 teamID)
{
	auto KBGM = Cast<AKFBBGameModeBase>(GetWorld()->GetAuthGameMode());
	if (KBGM)
	{
		KBGM->RegisterCoach(this, teamID);
		Field->TeamColorTileMaterials(teamID);
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

	DrawTileDebug(DeltaTime);
}

void AKFBB_CoachPC::NextTileDebugState()
{
	if (TileDebugState == EDebugTile::Show_None)
	{
		TileDebugState = EDebugTile::Show_TileIndex;
	}
	else if (TileDebugState == EDebugTile::Show_TileIndex)
	{
		TileDebugState = EDebugTile::Show_TileXY;
	}
	else if (TileDebugState == EDebugTile::Show_TileXY)
	{
		TileDebugState = EDebugTile::Show_TileTeamID;
	}
	else
	{
		TileDebugState = EDebugTile::Show_None;
	}
}

void AKFBB_CoachPC::DrawTileDebug(float DeltaTime)
{
	if (!Field) { return; }

	for (int i = 0; i < Field->Tiles.Num(); i++)
	{
		auto t = Field->Tiles[i];
		if (TileDebugState == EDebugTile::Show_TileIndex)
		{
			DrawDebugString(GetWorld(), t->TileLocation, FString::Printf(TEXT("%d"), t->TileIdx), nullptr, FColor::White, 0.01f);
		}
		else if (TileDebugState == EDebugTile::Show_TileXY)
		{
			DrawDebugString(GetWorld(), t->TileLocation, FString::Printf(TEXT("%d/%d"), t->TileX, t->TileY), nullptr, FColor::White, 0.01f);
		}
		else if (TileDebugState == EDebugTile::Show_TileTeamID)
		{
			DrawDebugString(GetWorld(), t->TileLocation, FString::Printf(TEXT("%d"), t->TileTeamID), nullptr, FColor::White, 0.01f);
		}
	}
}
