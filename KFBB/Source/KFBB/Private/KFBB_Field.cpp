// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_Field.h"
#include "DrawDebugHelpers.h"

// Sets default values
AKFBB_Field::AKFBB_Field()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AKFBB_Field::BeginPlay()
{
	Super::BeginPlay();

	if(Length == 0)		 Length			= 16;
	if(Width == 0)		 Width			= 11;
	if(TileSize == 0.f)  TileSize		= 64.f;
	if(EndZoneSize == 0) EndZoneSize	= 1;
	if(WideOutSize == 0) WideOutSize	= 3;

	auto FieldWidthDist  = TileSize * Width;
	auto FieldLengthDist = TileSize * Length;
	Origin = GetActorLocation() - FVector(FieldWidthDist * 0.5f, FieldLengthDist * 0.5f, 0.f);
	TileStep = FVector(TileSize, TileSize, 0.f);

	Map.AddZeroed(Length*Width);
	for (int y = 0; y < Length; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			int idx = GetIndexByXY(x, y);
			auto& tile = Map[idx];
			tile.Init(this, idx, x, y);
		}
	}
}

void AKFBB_Field::Destroyed()
{
	Super::Destroyed();
}

// Called every frame
void AKFBB_Field::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int i = 0; i < Map.Num(); i++)
	{
		auto& t = Map[i];
		t.DrawDebugTile();
	}
}

int AKFBB_Field::GetIndexByXY(int x, int y) const
{
	if (x < 0 || x >= Width || y < 0 || y >= Length)
		return -1;

	return (y * Width) + x;
}

FVector AKFBB_Field::GetFieldTileLocation(int x, int y) const
{
	int idx = GetIndexByXY(x, y);
	if (idx >= 0)
	{
		return Map[idx].TileLocation;
	}

	return FVector::ZeroVector;
}


FFieldTile::FFieldTile()
{
}

FFieldTile::~FFieldTile()
{
}

void FFieldTile::Init(AKFBB_Field* inField, int inIdx, int inX, int inY)
{
	Field = inField;
	idx = inIdx;
	x = inX;
	y = inY;

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
}

void FFieldTile::DrawDebugTile()
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
