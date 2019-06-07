




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

	Tiles = TArray<UKFBB_FieldTile*>();
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


