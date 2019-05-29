// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBBGameModeBase.h"

// Engine Includes
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"

// KFBB Includes
#include "KFBB_Field.h"
#include "KFBB_PlayerPawn.h"


void AKFBBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	CurrentGameTimer = GameDurationInSeconds;

	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}

	AKFBB_Field::AssignFieldActor(this, Field);
}

void AKFBBGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentGameTimer -= DeltaTime;
}

float AKFBBGameModeBase::GetRemainingGameTime() const
{
	return CurrentGameTimer;
}

int AKFBBGameModeBase::GetFieldWidth() const
{
	if (Field == nullptr)
		return -1;
	return Field->Width;
}

int AKFBBGameModeBase::GetFieldLength() const
{
	if (Field == nullptr)
		return -1;
	return Field->Length;
}

FVector AKFBBGameModeBase::GetFieldTileLocation(int x, int y) const
{
	if (Field == nullptr)
		return FVector::ZeroVector;

	return Field->GetFieldTileLocation(x, y);
}

void AKFBBGameModeBase::ResolveCollision(AKFBB_PlayerPawn* PawnA, AKFBB_PlayerPawn* PawnB)
{
	// PawnA = player already occupying tile
	// PawnB = player moving onto the tile, PawnB->PreviousTile will give direction of movement

	auto KnockDirA = Field->GetTileDir(PawnB->PreviousTile, PawnB->CurrentTile);
	auto KnockDirB = Field->GetTileDir(PawnB->CurrentTile, PawnB->PreviousTile);
	
// 	KnockDirA.x = 0;
// 	KnockDirB.y = 0;

	PawnA->KnockDown(KnockDirA);
	PawnB->KnockDown(KnockDirB);
}
