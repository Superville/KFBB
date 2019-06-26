// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_FieldTile.h"
#include "KFBB_Field.h"
#include "KFBB_PlayerPawn.h"
#include "KFBB_Ball.h"
#include "KFBB.h"
#include "DrawDebugHelpers.h"

void UKFBB_FieldTile::Init(AKFBB_Field* inField, int inIdx)
{
	Field = inField;
	TileIdx = inIdx;
	TileX = Field->GetXByIndex(TileIdx);
	TileY = Field->GetYByIndex(TileIdx);

	TileLocation = Field->Origin + FVector(Field->TileSize * TileX, Field->TileSize * TileY, 0.f) + (Field->TileStep * 0.5f);

	if ((TileY - Field->EndZoneSize) < 0)
	{
		Field->EndZoneTiles_Team0.Add(this);
		bEndZone_Team0 = true;
	}
	if ((TileY + Field->EndZoneSize) >= Field->Length)
	{
		Field->EndZoneTiles_Team1.Add(this);
		bEndZone_Team1 = true;
	}
	if ((TileX - Field->WideOutSize) < 0 || (TileX + Field->WideOutSize) >= Field->Width)
	{
		bWideOut = true;
	}
	auto HalfLength = Field->Length / 2;
	if (TileY == HalfLength || TileY == (HalfLength - 1))
	{
		bScrimmageLine = true;
	}

	TileTeamID = (TileY < Field->Length / 2) ? 1 : 0;

	SetStaticMesh(Field->TileMesh);
	SetWorldLocation(TileLocation - (Field->TileStep*0.5f));
	auto bounds = Field->TileMesh->GetBounds().BoxExtent * 2;
	auto scale = FVector(Field->TileSize, Field->TileSize, 20.f) / bounds;
	SetRelativeScale3D(scale);
	
	// Set visual appearance of the tile
	if (bEndZone_Team0 || bEndZone_Team1) SetMaterial(0, Field->Mat_EndZone);
	else if (bWideOut) SetMaterial(0, Field->Mat_WideOut);
	else if (bScrimmageLine) SetMaterial(0, Field->Mat_Scrimmage);
	else SetMaterial(0, Field->Mat_Field);

	SetCollisionResponseToChannel(ECC_FieldTrace, ECollisionResponse::ECR_Block);
}

bool UKFBB_FieldTile::RegisterActor(AActor* a)
{
	RegisteredActors.Add(a);
	return true;
}

bool UKFBB_FieldTile::UnRegisterActor(AActor* a)
{
	int idx = RegisteredActors.Remove(a);
	return (idx > 0);
}

bool UKFBB_FieldTile::HasPlayer() const
{
	for (int i = 0; i < RegisteredActors.Num(); ++i)
	{
		if (RegisteredActors[i]->IsA(AKFBB_PlayerPawn::StaticClass()))
		{
			return true;
		}
	}
	return false;
}

bool UKFBB_FieldTile::HasBall() const
{
	for (int i = 0; i < RegisteredActors.Num(); ++i)
	{
		if (RegisteredActors[i]->IsA(AKFBB_Ball::StaticClass()))
		{
			return true;
		}
	}
	return false;
}

AKFBB_PlayerPawn* UKFBB_FieldTile::GetPlayer() const
{
	for (int i = 0; i < RegisteredActors.Num(); ++i)
	{
		AKFBB_PlayerPawn* p = Cast<AKFBB_PlayerPawn>(RegisteredActors[i]);
		if (p != nullptr)
		{
			return p;
		}
	}
	return nullptr;
}

AKFBB_Ball* UKFBB_FieldTile::GetBall() const
{
	for (int i = 0; i < RegisteredActors.Num(); ++i)
	{
		AKFBB_Ball* b = Cast<AKFBB_Ball>(RegisteredActors[i]);
		if (b != nullptr)
		{
			return b;
		}
	}
	return nullptr;
}

UKFBB_FieldTile* UKFBB_FieldTile::GetAdjacentTile(FTileDir dir)
{
	return Field != nullptr ? Field->GetAdjacentTile(this, dir) : nullptr;
}

bool UKFBB_FieldTile::GetTileVerts(FTileDir dir, TArray<FVector>& out_ResultList)
{
	if (!Field || !dir.IsCardinalDir())
	{ 
		return false; 
	}

	//todo - store vert offsets from TileLocation at creation time
	if (dir.x != 0)
	{
		out_ResultList.Add(TileLocation + FVector(dir.x * Field->TileSize,  Field->TileSize, 0) * 0.5f);
		out_ResultList.Add(TileLocation + FVector(dir.x * Field->TileSize, -Field->TileSize, 0) * 0.5f);
	}
	else
	{
		out_ResultList.Add(TileLocation + FVector( Field->TileSize, dir.y * Field->TileSize, 0) * 0.5f);
		out_ResultList.Add(TileLocation + FVector(-Field->TileSize, dir.y * Field->TileSize, 0) * 0.5f);
	}

	return true;
}

FVector UKFBB_FieldTile::GetTileSize2D() const
{
	return Field != nullptr ? FVector(Field->TileSize, Field->TileSize, 0.f) : FVector::ZeroVector;
}

float UKFBB_FieldTile::GetTileSize() const
{
	return Field != nullptr ? Field->TileSize : 0.f;
}

void UKFBB_FieldTile::DrawCooldownTimer(float Duration, float TimeRemaining, FColor DrawColor)
{
	if (!Field || Duration <= 0.f) { return; }
	
	float DurationPct = TimeRemaining / Duration;

	float sign = (/* team0 */ false) ? -1 : 1;

	FVector FieldDir = Field->GetActorRightVector();
	FVector Shift = FieldDir * Field->TileSize * 0.5f * (1.f - DurationPct);
	FVector Extent = (GetTileSize2D() * 0.5f) - Shift;
	FVector Loc = TileLocation + FVector(0, 0, Field->TileHeight) + (Shift * sign);

	DrawDebugSolidBox(GetWorld(), Loc, Extent, DrawColor);
}

void UKFBB_FieldTile::DrawDebugTile() const
{
	if (Field == nullptr)
		return;

	auto MyWorld = Field->GetWorld();
	FColor color = GetDebugColor();

	DrawDebugBox(MyWorld, TileLocation, FVector(Field->TileSize, Field->TileSize, 0) * 0.5f, color);
	DrawDebugLine(MyWorld, TileLocation, TileLocation + FVector(0, 0, 5), color);
}

void UKFBB_FieldTile::DrawDebugTile(FVector offset) const
{
	if (Field == nullptr)
		return;

	auto MyWorld = Field->GetWorld();
	FColor color = GetDebugColor();

	DrawDebugBox(MyWorld, TileLocation + offset, FVector(Field->TileSize, Field->TileSize, 0) * 0.5f, color);

	if (HasBall())
	{
		DrawDebugCircle(MyWorld, TileLocation + offset, Field->TileSize * 0.5f, 8, color);
	}
}

void UKFBB_FieldTile::DrawDebugTileOverride(FVector offset, float scale, FColor c, float thickness) const
{
	if (Field == nullptr)
		return;

	auto MyWorld = Field->GetWorld();

	DrawDebugBox(MyWorld, TileLocation + offset, FVector(Field->TileSize, Field->TileSize, 0) * scale, c, false, -1.f, 0, thickness);
}

FColor UKFBB_FieldTile::GetDebugColor() const
{
	FColor color = FColor::Green;
	auto Ball = GetBall();
	if (Ball)
	{
		color = Ball->CanBePickedUp() ? FColor::White : FColor::Blue;
	}	
	else if (bEndZone_Team0 || bEndZone_Team1) color = FColor::Red;
	else if (bWideOut) color = FColor::Orange;
	else if (bScrimmageLine) color = FColor::Cyan;
	return color;
}
