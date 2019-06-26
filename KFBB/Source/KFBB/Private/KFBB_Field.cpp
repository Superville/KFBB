




// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"


FTileDir FTileDir::ConvertToTileDir(FVector2D v)
{
	v = v.GetSafeNormal();
	if (FMath::Abs(v.X) > 0.7f)
	{
		v.X = (v.X < 0) ? -1 : 1;
	}
	else
	{
		v.X = 0;
	}

	if (FMath::Abs(v.Y) > 0.7f)
	{
		v.Y = (v.Y < 0) ? -1 : 1;
	}
	else
	{
		v.Y = 0;
	}

	return FTileDir(v.X, v.Y);
}

// Sets default values
AKFBB_Field::AKFBB_Field()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AKFBB_Field::AKFBB_Field(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Root = ObjectInitializer.CreateAbstractDefaultSubobject<USceneComponent>(this, TEXT("Root"));
	RootComponent = Root;

//	Tiles = TArray<UKFBB_FieldTile*>();
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

	TileDirections.Add(FTileDir(1, 0));
	TileDirections.Add(FTileDir(1, 1));
	TileDirections.Add(FTileDir(0, 1));
	TileDirections.Add(FTileDir(-1, 1));
	TileDirections.Add(FTileDir(-1, 0));
	TileDirections.Add(FTileDir(-1, -1));
	TileDirections.Add(FTileDir(0, -1));
	TileDirections.Add(FTileDir(1, -1));

	auto TileList = GetComponentsByClass(UKFBB_FieldTile::StaticClass());
	for (int i = 0; i < TileList.Num(); ++i)
	{
		UKFBB_FieldTile* t = Cast<UKFBB_FieldTile>(TileList[i]);
		if (t != nullptr)
		{
			Tiles.Insert(t, t->TileIdx);
			t->Init(this, t->TileIdx);
		}
	}
}

void AKFBB_Field::Destroyed()
{
	Super::Destroyed();
}

void AKFBB_Field::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (Length == 0)		Length = 16;
	if (Width == 0)			Width = 11;
	if (TileSize == 0.f)	TileSize = 48.f;
	if (EndZoneSize == 0)	EndZoneSize = 1;
	if (WideOutSize == 0)	WideOutSize = 3;

	if ((Length % 2) != 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Field LENGTH must be an EVEN number!!!!"));
	}
	if ((Width % 2) != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Field WIDTH must be an ODD number!!!!"));
	}

	Tiles.Empty();

	auto FieldWidthDist = TileSize * Width;
	auto FieldLengthDist = TileSize * Length;
	Origin = GetActorLocation() - FVector(FieldWidthDist * 0.5f, FieldLengthDist * 0.5f, 0.f);
	TileStep = FVector(TileSize, TileSize, 0.f);

	FAttachmentTransformRules atr(EAttachmentRule::KeepWorld, true);
	int totalTiles = Width * Length;
	for (int i = 0; i < totalTiles; ++i)
	{
		UKFBB_FieldTile* t = NewObject<UKFBB_FieldTile>(this);
		t->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
		t->RegisterComponent();
		t->Init(this, i);

		Tiles.Add(t);
	}
}

bool AKFBB_Field::AreAdjacentTiles(UKFBB_FieldTile* a, UKFBB_FieldTile* b)
{
	if (!a || !b) { return false; }

	return (FMath::Abs(a->TileX - b->TileX) <= 1) && (FMath::Abs(a->TileY - b->TileY) <= 1);
}

UKFBB_FieldTile* AKFBB_Field::GetAdjacentTile(UKFBB_FieldTile* tile, FTileDir dir)
{
	int idx = GetIndexByXY(tile->TileX + dir.x, tile->TileY + dir.y);
	if (idx < 0) { return nullptr; }
	return Tiles[idx];
}

/* return FTileDir from a to b*/
FTileDir AKFBB_Field::GetTileDir(UKFBB_FieldTile* a, UKFBB_FieldTile* b)
{
	if (!a || !b) { return FTileDir(0, 0); }

	int xDelta = b->TileX - a->TileX;
	int yDetla = b->TileY - a->TileY;

	return FTileDir(xDelta, yDetla);
}

// Called every frame
void AKFBB_Field::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

int AKFBB_Field::GetIndexByXY(int x, int y) const
{
	if (x < 0 || x >= Width || y < 0 || y >= Length)
	{
		return -1;
	}

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

UKFBB_FieldTile* AKFBB_Field::GetTileByXY(int x, int y) const
{
	int TileIdx = GetIndexByXY(x, y);
	return (Tiles.IsValidIndex(TileIdx)) ? Tiles[TileIdx] : nullptr;	
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

void AKFBB_Field::TeamColorTileMaterials(int TeamIdx)
{
	if (TeamIdx < 0 || TeamIdx > 1) { return; }

	for (int i = 0; i < EndZoneTiles_Team0.Num(); i++)
	{
		auto t = EndZoneTiles_Team0[i];
		if (t)
		{
			t->SetMaterial(0, TeamIdx == 0 ? MatInst_EndZone_Blue : MatInst_EndZone_Red);
		}
	}
	for (int i = 0; i < EndZoneTiles_Team1.Num(); i++)
	{
		auto t = EndZoneTiles_Team1[i];
		if (t)
		{
			t->SetMaterial(0, TeamIdx == 1 ? MatInst_EndZone_Blue : MatInst_EndZone_Red);
		}
	}
}

FTileDir AKFBB_Field::GetScatterDirection(short centerX, short centerY, int cone)
{
	FTileDir c(centerX, centerY);
	int idx = FMath::Max(TileDirections.Find(c), 0);
	int offset = FMath::RandRange(FMath::Max(-3, -cone), cone);
	idx += offset;
	while (idx < 0) { idx += TileDirections.Num(); }
	while (idx >= TileDirections.Num()) { idx -= TileDirections.Num(); }

	return TileDirections[idx];
}

TArray<UKFBB_FieldTile*> AKFBB_Field::GetListOfTilesInRange(UKFBB_FieldTile* StartTile, int Range)
{
	TArray<UKFBB_FieldTile*> ResultList;
	if (Range <= 0 || !StartTile) { return ResultList; }

	int currRange = 0;
	TArray<UKFBB_FieldTile*> currList;
	TArray<UKFBB_FieldTile*> nextList;
	currList.Add(StartTile);

	while (currRange < Range && currList.Num() > 0)
	{
		for (int i = 0; i < currList.Num(); i++)
		{
			UKFBB_FieldTile* curr = currList[i];
			for (int dir = 0; dir < TileDirections.Num(); dir++)
			{
				UKFBB_FieldTile* n = GetAdjacentTile(curr, TileDirections[dir]);
				if (n != nullptr && ResultList.Find(n) == INDEX_NONE)
				{
					ResultList.Add(n);
					nextList.Add(n);
				}
			}
		}
		
		currList = nextList;
		nextList.Empty();

		currRange++;
	}

	if (ResultList.Num() > 0)
	{
		ResultList.Add(StartTile);
	}

	return ResultList;
}

TArray<FVector> AKFBB_Field::GetExternalEdgeVerts(TArray<UKFBB_FieldTile*>& TileList)
{
	TArray<FVector> ResultList;
	TArray<FVector> currVerts;
	for (int i = 0; i < TileList.Num(); i++)
	{
		UKFBB_FieldTile* curr = TileList[i];
		if(curr == nullptr) { continue; }
		// Get tile in each direction
		for (int j = 0; j < TileDirections.Num(); j++)
		{
			FTileDir dir = TileDirections[j];
			if (!dir.IsCardinalDir()) { continue; }

			UKFBB_FieldTile* neighbor = GetAdjacentTile(curr, dir);
			// If tile in current direction is NOT in the list
			if (TileList.Find(neighbor) == INDEX_NONE)
			{
				// Add the two verts from the edge of the current tile in that direction
				if (curr->GetTileVerts(dir, currVerts))
				{
					ResultList.Append(currVerts);
				}
				
				currVerts.Empty();
			}
		}
	}

	return ResultList;
}


