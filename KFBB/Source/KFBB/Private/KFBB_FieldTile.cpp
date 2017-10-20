// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_FieldTile.h"
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
