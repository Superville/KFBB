// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBBGameModeBase.h"
#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"


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

	Field = nullptr;
	for (TActorIterator<AKFBB_Field> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (Field != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("AKFBBGameModeBase::BeginPlay - Found duplicate AKFBB_Field Actor - %s"), *((*ActorItr)->GetName()));
			continue;
		}
		
		Field = *ActorItr;
	}
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

	int idx = Field->GetIndexByXY(x, y);
	if (idx >= 0)
	{
		auto& tile = Field->Map[idx];
		return tile.TileLocation;
	}

	return FVector::ZeroVector;
}
