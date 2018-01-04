// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_Ball.h"
#include "KFBB_PlayerPawn.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "KFBB.h"
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
	if(!IsMoving() && TimeSinceLastFumble() > 1.f)
	{
		StopMovement();
	}

	//debug
	DrawDebugCurrentTile();

	//test
	Test();
}

void AKFBB_Ball::RegisterWithField()
{
	bOnGround = false;
	if (OwningPlayer != nullptr)
	{
		RegisterWithTile(OwningPlayer->CurrentTile);
	}
	else
	{
		auto MyWorld = GetWorld();
		FVector Loc = GetActorLocation();

		FHitResult Hit;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		if (MyWorld->LineTraceSingleByChannel(Hit, Loc, Loc + FVector(0, 0, -1000), ECollisionChannel::ECC_FieldTrace, CollisionParams))
		{
			//test
			auto hc = Hit.GetComponent();

			UKFBB_FieldTile* Tile = Cast<UKFBB_FieldTile>(Hit.GetComponent());
			RegisterWithTile(Tile);

			auto distFromGround = (Loc - Hit.Location).Size();
			constexpr float distFromGroundThreshold = 10.f;
			if (distFromGround < distFromGroundThreshold)
			{
				bOnGround = true;
			}
		}
	}
}

void AKFBB_Ball::RegisterWithTile(class UKFBB_FieldTile* Tile)
{
	//test
	if (Tile == nullptr)
	{
		int i = 0;
	}

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

void AKFBB_Ball::FumbleBall(UKFBB_FieldTile* DestTile)
{
	auto smc = Cast<UStaticMeshComponent>(GetRootComponent());
	if (smc != nullptr)
	{
		FVector ballVel = (DestTile->TileLocation - CurrentTile->TileLocation).GetSafeNormal2D() * 100;
		ballVel.Z += 250;

		FVector ballAngVel = FMath::VRand() * (FMath::FRandRange(-10.f, 10.f));

		smc->SetPhysicsLinearVelocity(ballVel);
		smc->SetPhysicsAngularVelocityInDegrees(ballAngVel);
	}

	LastFumbleTime = GetWorld()->TimeSeconds;
}

float AKFBB_Ball::TimeSinceLastFumble() const
{
	return (GetWorld()->TimeSeconds - LastFumbleTime);
}

bool AKFBB_Ball::IsMoving()
{
	if (bOnGround)
	{
		auto v = GetVelocity();
		constexpr float ZVelocityThreshold = 5.f;
		if (FMath::Abs(v.Z) < 5.f)
		{
			return false;
		}
	}

	return true;
}

void AKFBB_Ball::StopMovement()
{
	auto smc = Cast<UStaticMeshComponent>(GetRootComponent());
	if (smc != nullptr)
	{
		smc->SetPhysicsLinearVelocity(FVector::ZeroVector);
		smc->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
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