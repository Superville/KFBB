// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_FieldTile.h"
#include "KFBB_Field.h"
#include "KFBB_PlayerPawn.h"
#include "DrawDebugHelpers.h"

void UKFBB_FieldTile::Init(AKFBB_Field* inField, int inIdx)
{
	Field = inField;
	idx = inIdx;
	x = Field->GetXByIndex(idx);
	y = Field->GetYByIndex(idx);

	TileLocation = Field->Origin + FVector(Field->TileSize * x, Field->TileSize * y, 0.f) + (Field->TileStep * 0.5f);

	if ((y - Field->EndZoneSize) < 0 || (y + Field->EndZoneSize) >= Field->Length)
	{
		bEndZone = true;
	}
	if ((x - Field->WideOutSize) < 0 || (x + Field->WideOutSize) >= Field->Width)
	{
		bWideOut = true;
	}
	auto HalfLength = Field->Length / 2;
	if (y == HalfLength || y == (HalfLength - 1))
	{
		bScrimmageLine = true;
	}

	SetWorldLocation(TileLocation - (Field->TileStep*0.5f));
	SetRelativeScale3D(FVector(Field->TileSize / 400.f, Field->TileSize / 400.f, 1.f));

	// Set visual appearance of the tile
	if (bEndZone) SetMaterial(0, Field->Mat_EndZone);
	else if (bWideOut) SetMaterial(0, Field->Mat_WideOut);
	else if (bScrimmageLine) SetMaterial(0, Field->Mat_Scrimmage);
	else SetMaterial(0, Field->Mat_Field);

//	SetCanEverAffectNavigation(false);
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

bool UKFBB_FieldTile::HasPlayer()
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

bool UKFBB_FieldTile::HasBall()
{
	return false;
}

AKFBB_PlayerPawn* UKFBB_FieldTile::GetPlayer()
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

AActor* UKFBB_FieldTile::GetBall()
{
	return nullptr;
}

void UKFBB_FieldTile::DrawDebugTile() const
{
	if (Field == nullptr)
		return;

	auto MyWorld = Field->GetWorld();

	FColor color = FColor::Green;
	if (bEndZone) color = FColor::Red;
	else if (bWideOut) color = FColor::Orange;
	else if (bScrimmageLine) color = FColor::Cyan;

	DrawDebugBox(MyWorld, TileLocation, FVector(Field->TileSize, Field->TileSize, 0) * 0.5f, color);
	DrawDebugLine(MyWorld, TileLocation, TileLocation + FVector(0, 0, 5), color);
}
