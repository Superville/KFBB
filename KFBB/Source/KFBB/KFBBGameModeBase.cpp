// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBBGameModeBase.h"
#include "KFBB_Field.h"
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
