// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_Ball.h"

// Engine Includes
#include "AbilitySystemComponent.h"
#include "UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

// KFBB Includes
#include "KFBB_PlayerPawn.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "KFBB.h"
#include "KFBB_BallMovementComponent.h"

// Sets default values
AKFBB_Ball::AKFBB_Ball()
{
	BallSMC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallSMC"));
	SetRootComponent(BallSMC);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(FName("AbilitySystemComp"));

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
	if (IsMoving() && TimeSinceLastFumble() > 1.f)
	{
		StopMovement();
	}
	if(!IsMoving() )
	{
		AdjustBallToTileCenter(DeltaTime);
	}
	UpdateCanBePickedUp();
	
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
	if (CurrentTile != Tile)
	{
		if (CurrentTile != nullptr) { CurrentTile->UnRegisterActor(this); }
		CurrentTile = Tile;
		if (CurrentTile != nullptr) { CurrentTile->RegisterActor(this); }
	}
}

void AKFBB_Ball::RegisterWithPlayer(AKFBB_PlayerPawn* P)
{
	if (!P || !P->AbilitySystemComponent) { return; }
	if (OwningPlayer) { return; }

	OwningPlayer = P;
	OwningPlayer->Ball = this;

	OwningPlayer->AbilitySystemComponent->AddLooseGameplayTag(OwningPlayer->HasBallTag);
	OwningPlayer->AbilitySystemComponent->SetTagMapCount(OwningPlayer->HasBallTag, 1);
}

void AKFBB_Ball::UnRegisterWithPlayer()
{
	if (OwningPlayer == nullptr || !OwningPlayer->AbilitySystemComponent) { return; }

	OwningPlayer->AbilitySystemComponent->RemoveLooseGameplayTag(OwningPlayer->HasBallTag);
	OwningPlayer->Ball = nullptr;
	OwningPlayer = nullptr;
}

void AKFBB_Ball::FumbleBall(UKFBB_FieldTile* DestTile)
{
	UnRegisterWithPlayer();

	const FDetachmentTransformRules dtr(EDetachmentRule::KeepWorld, false);
	DetachFromActor(dtr);
	

	if (BallSMC != nullptr)
	{
		FVector ballVel = (DestTile->TileLocation - CurrentTile->TileLocation).GetSafeNormal2D() * 100;
		ballVel.Z += 250;

		FVector ballAngVel = FMath::VRand() * (FMath::FRandRange(-10.f, 10.f));
		
		BallSMC->SetSimulatePhysics(true);
		BallSMC->SetPhysicsLinearVelocity(ballVel);
		BallSMC->SetPhysicsAngularVelocityInDegrees(ballAngVel);
	}

	LastFumbleTime = GetWorld()->TimeSeconds;
}

float AKFBB_Ball::TimeSinceLastFumble() const
{
	return (GetWorld()->TimeSeconds - LastFumbleTime);
}

bool AKFBB_Ball::CanBePickedUp() const
{
	return (AbilitySystemComponent && AbilitySystemComponent->GetTagCount(CanBePickedUpTag) > 0);
}

void AKFBB_Ball::UpdateCanBePickedUp()
{
	if (AbilitySystemComponent)
	{
		if (IsPossessed() || IsMoving() || TimeSinceLastFumble() <= 1.f)
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(CanBePickedUpTag);
		}
		else
		{
			AbilitySystemComponent->AddLooseGameplayTag(CanBePickedUpTag);
			AbilitySystemComponent->SetTagMapCount(CanBePickedUpTag, 1);
		}
	}
}

bool AKFBB_Ball::IsPossessed() const
{
	return (OwningPlayer != nullptr);
}

bool AKFBB_Ball::IsMoving() const
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

void AKFBB_Ball::AdjustBallToTileCenter(float DeltaTime)
{
	if (CurrentTile != nullptr)
	{
		FVector BallLocation = GetActorLocation();
		FVector BallToTileCenter = (CurrentTile->TileLocation - BallLocation);
		float DistToTileCenter = BallToTileCenter.Size2D();
		if (DistToTileCenter > 5.f)
		{
			float AdjustToCenterSpeed = 30.f;
			SetActorLocation(BallLocation + BallToTileCenter.GetSafeNormal2D() * AdjustToCenterSpeed * DeltaTime);
		}
	}
}

UAbilitySystemComponent* AKFBB_Ball::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
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