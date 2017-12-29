// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"

// Sets default values
AKFBB_Field::AKFBB_Field()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

bool AKFBB_Field::AssignFieldActor(AActor* src, AKFBB_Field*& ptrField)
{
	if (src == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AKFBB_Field::AssignFieldActor - NULL Src Actor"));
	}

	ptrField = nullptr;
	for (TActorIterator<AKFBB_Field> ActorItr(src->GetWorld()); ActorItr; ++ActorItr)
	{
		if (ptrField != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("AKFBB_Field::AssignFieldActor - Found duplicate AKFBB_Field Actor - %s (Called by %s)"), *((*ActorItr)->GetName()), *src->GetName());
			continue;
		}

		ptrField = *ActorItr;
	}

	if (ptrField == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("AKFBB_Field::AssignFieldActor - Failed to find AKFBB_Field Actor (Called by %s)"), *src->GetName());
	}
	
	return (ptrField != nullptr);
}

// Called when the game starts or when spawned
void AKFBB_Field::BeginPlay()
{
	Super::BeginPlay();

	ScatterDirections.Add(FScatterDir(1, 0));
	ScatterDirections.Add(FScatterDir(1, 1));
	ScatterDirections.Add(FScatterDir(0, 1));
	ScatterDirections.Add(FScatterDir(-1, 1));
	ScatterDirections.Add(FScatterDir(-1, 0));
	ScatterDirections.Add(FScatterDir(-1, -1));
	ScatterDirections.Add(FScatterDir(0, -1));
	ScatterDirections.Add(FScatterDir(1, -1));

	auto TileList = GetComponentsByClass(UKFBB_FieldTile::StaticClass());
	for (int i = 0; i < TileList.Num(); ++i)
	{
		UKFBB_FieldTile* t = Cast<UKFBB_FieldTile>(TileList[i]);
		if (t != nullptr)
		{
			Tiles.Insert(t, t->idx);
		}
	}
}

void AKFBB_Field::Destroyed()
{
	Super::Destroyed();
}

void AKFBB_Field::Init()
{
	if (Length == 0)		Length = 16;
	if (Width == 0)			Width = 11;
	if (TileSize == 0.f)	TileSize = 48.f;
	if (EndZoneSize == 0)	EndZoneSize = 1;
	if (WideOutSize == 0)	WideOutSize = 3;

	auto FieldWidthDist = TileSize * Width;
	auto FieldLengthDist = TileSize * Length;
	Origin = GetActorLocation() - FVector(FieldWidthDist * 0.5f, FieldLengthDist * 0.5f, 0.f);
	TileStep = FVector(TileSize, TileSize, 0.f);

	auto TileList = GetComponentsByClass(UKFBB_FieldTile::StaticClass());
	for (int i = 0; i < TileList.Num(); ++i)
	{
		UKFBB_FieldTile* t = Cast<UKFBB_FieldTile>(TileList[i]);
		if (t != nullptr)
		{
			t->Init(this, i);
		}		
	}
}

UKFBB_FieldTile* AKFBB_Field::GetAdjacentTile(UKFBB_FieldTile* tile, FScatterDir dir)
{
	int idx = GetIndexByXY(tile->x + dir.x, tile->y + dir.y);
	if(idx < 0) { return nullptr; }
	return Tiles[idx];	
}

// Called every frame
void AKFBB_Field::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int AKFBB_Field::GetIndexByXY(int x, int y) const
{
	if (x < 0 || x >= Width || y < 0 || y >= Length)
		return -1;

	return (y * Width) + x;
}

int AKFBB_Field::GetXByIndex(int idx) const
{
	if (idx < 0 || idx >= (Width * Length))
		return -1;

	return idx % Width;
}

int AKFBB_Field::GetYByIndex(int idx) const
{
	if (idx < 0 || idx >= (Width * Length))
		return -1;

	return idx / Width;
}

FVector AKFBB_Field::GetFieldTileLocation(int x, int y) const
{
	int idx = GetIndexByXY(x, y);

	auto TileList = GetComponentsByClass(UKFBB_FieldTile::StaticClass());
	if (idx >= 0 && idx < TileList.Num())
	{
		UKFBB_FieldTile* t = Cast<UKFBB_FieldTile>(TileList[idx]);
		return t->TileLocation;
	}
	return FVector::ZeroVector;
}

FScatterDir AKFBB_Field::GetScatterDirection(short centerX, short centerY, int cone)
{
	FScatterDir c(centerX, centerY);
	int idx = FMath::Max(ScatterDirections.Find(c),0);
	int offset = FMath::RandRange(FMath::Max(-3, -cone), cone);
	idx += offset;
	while (idx < 0) { idx += ScatterDirections.Num(); }
	while (idx >= ScatterDirections.Num()) { idx -= ScatterDirections.Num(); }

	return ScatterDirections[idx];
}

