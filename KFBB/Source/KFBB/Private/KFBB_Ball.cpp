// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_Ball.h"
#include "KFBB_PlayerPawn.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

#include "KFBB_BallMovementComponent.h"

// Sets default values
AKFBB_Ball::AKFBB_Ball()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AKFBB_Ball::BeginPlay()
{
	Super::BeginPlay();
	
	AKFBB_Field::AssignFieldActor(this, Field);
	RegisterWithField();
}

// Called every frame
void AKFBB_Ball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RegisterWithField();

	//debug
	DrawDebugCurrentTile();

	//test
	Test();
}

void AKFBB_Ball::RegisterWithField()
{
	if (OwningPlayer != nullptr)
	{
		RegisterWithTile(OwningPlayer->CurrentTile);
	}
	else
	{
		auto MyWorld = GetWorld();
		FVector Loc = GetActorLocation();

		FHitResult Hit;
		if (MyWorld->LineTraceSingleByChannel(Hit, Loc, Loc + FVector(0, 0, -1000), ECollisionChannel::ECC_Visibility))
		{
			UKFBB_FieldTile* Tile = Cast<UKFBB_FieldTile>(Hit.GetComponent());
			RegisterWithTile(Tile);
		}
	}
}

void AKFBB_Ball::RegisterWithTile(class UKFBB_FieldTile* Tile)
{
	if (CurrentTile != Tile)
	{
		if (CurrentTile != nullptr) { CurrentTile->UnRegisterActor(this); }
		CurrentTile = Tile;
		if (CurrentTile != nullptr) { CurrentTile->RegisterActor(this); }
	}
}

void AKFBB_Ball::RegisterWithPlayer(AKFBB_PlayerPawn* P)
{
	if (OwningPlayer != nullptr)
	{
		//error!
		return;
	}

	OwningPlayer = P;
	OwningPlayer->Ball = this;
}

void AKFBB_Ball::UnRegisterWithPlayer()
{
	if (OwningPlayer == nullptr)
	{
		// error!
		return;
	}

	OwningPlayer->Ball = nullptr;
	OwningPlayer = nullptr;
}

bool AKFBB_Ball::IsMoving()
{
	auto v = GetVelocity();
	return (v.Size() > 100.f);
}

void AKFBB_Ball::DrawDebugCurrentTile() const
{
	if (Field == nullptr || CurrentTile == nullptr)
		return;

	CurrentTile->DrawDebugTile(FVector(0, 0, 2));
}

void AKFBB_Ball::Test()
{
	auto bmc = Cast<UKFBB_BallMovementComponent>(GetComponentByClass(UKFBB_BallMovementComponent::StaticClass()));
	bmc->Test();
}