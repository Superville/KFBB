// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_PlayerPawn.h"
#include "KFBB_PlayerController.h"
#include "KFBB_CoachPC.h"
#include "KFBB_Field.h"
#include "DrawDebugHelpers.h"

// Sets default values
AKFBB_PlayerPawn::AKFBB_PlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AKFBB_PlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	AKFBB_Field::AssignFieldActor(this, Field);
	RegisterWithField();
	ClearCooldownTimer();
}

// Called every frame
void AKFBB_PlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RegisterWithField();
	if(IsPlayerOnCooldown())
	{
		CooldownTimer -= DeltaTime;
	}

	//debug
	DrawDebugCurrentTile();
}

// Called to bind functionality to input
void AKFBB_PlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AKFBB_PlayerPawn::RegisterWithField()
{
	auto MyWorld = GetWorld();
	FVector Loc = GetActorLocation();

	FHitResult Hit;
	if (MyWorld->LineTraceSingleByChannel(Hit, Loc, Loc + FVector(0,0,-1000), ECollisionChannel::ECC_Visibility))
	{
		UKFBB_FieldTile* Tile = Cast<UKFBB_FieldTile>(Hit.GetComponent());
		RegisterWithTile(Tile);
	}
}

void AKFBB_PlayerPawn::RegisterWithTile(class UKFBB_FieldTile* Tile)
{
	if (CurrentTile != Tile)
	{
		if (CurrentTile != nullptr) { CurrentTile->UnRegisterActor(this); }
		CurrentTile = Tile;
		if (CurrentTile != nullptr) { CurrentTile->RegisterActor(this); }
	}
}

void AKFBB_PlayerPawn::ClearCooldownTimer() { CooldownTimer = 0.f; }
void AKFBB_PlayerPawn::ResetCooldownTimer() { CooldownTimer = 3.f; }
bool AKFBB_PlayerPawn::IsPlayerOnCooldown() { return (CooldownTimer > 0.f); }

bool AKFBB_PlayerPawn::CanAcceptCommand()
{
	AAIController* ai = Cast<AAIController>(Controller);
	if(ai != nullptr && ai->GetMoveStatus() != EPathFollowingStatus::Idle)
	{
		return false;
	}
	
	if( IsPlayerOnCooldown())
	{
		return false;
	}

	return true;
}

void AKFBB_PlayerPawn::NotifyCommandFailed()
{
	auto MyWorld = GetWorld();
	FColor color = FColor::Red;

	AAIController* ai = Cast<AAIController>(Controller);
	if (ai != nullptr && ai->GetMoveStatus() != EPathFollowingStatus::Idle)
	{
		color = FColor::Yellow;
	}
		
	DrawDebugBox(MyWorld, GetActorLocation(), FVector(20, 20, 20), color, false, 1.f);
}

void AKFBB_PlayerPawn::NotifyReachedDestination()
{
	ResetCooldownTimer();

	auto MyWorld = GetWorld();
	FColor color = FColor::Green;
	DrawDebugBox(MyWorld, GetActorLocation(), FVector(20, 20, 20), color, false, 1.f);
}

void AKFBB_PlayerPawn::DrawDebugCurrentTile()
{
	if (Field == nullptr || CurrentTile == nullptr)
		return;

	auto MyWorld = GetWorld();

	FColor color = FColor::Green;
	if (CurrentTile->bEndZone) color = FColor::Red;
	else if (CurrentTile->bWideOut) color = FColor::Orange;
	else if (CurrentTile->bScrimmageLine) color = FColor::Cyan;

	DrawDebugBox(MyWorld, CurrentTile->TileLocation + FVector(0, 0, 2), FVector(Field->TileSize, Field->TileSize, 0) * 0.5f, color, false, 1.f);
}