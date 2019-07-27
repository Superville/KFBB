// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_Ball.h"

// Engine Includes
#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

// KFBB Includes
#include "KFBB_GameplayAbility.h"
#include "KFBB_PlayerPawn.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "KFBB.h"
#include "KFBB_BallMovementComponent.h"
#include "Game/KFBB_GameState.h"

// Sets default values
AKFBB_Ball::AKFBB_Ball()
{
	BallSMC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallSMC"));
	SetRootComponent(BallSMC);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(FName("AbilitySystemComp"));

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void AKFBB_Ball::BeginPlay()
{
	Super::BeginPlay();
	
	AKFBB_Field::AssignFieldActor(this, Field);
	RegisterWithField();

	auto KFGS = Cast<AKFBB_GameState>(GetWorld()->GetGameState());
	if (KFGS)
	{
		KFGS->RegisterBall(this);
	}

	InitAbilities();
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ApplyGameplayEffectToSelf(GE_BallState_Free.GetDefaultObject(), 1, FGameplayEffectContextHandle());
	}
}

void AKFBB_Ball::GrantAbility(TSubclassOf<UKFBB_GameplayAbility> Ability)
{
	if (AbilitySystemComponent && Ability && HasAuthority())
	{
		FGameplayAbilitySpecDef SpecDef = FGameplayAbilitySpecDef();
		SpecDef.Ability = Ability;
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(SpecDef, 1);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (Ability.GetDefaultObject()->bAutoActivate)
		{
			AbilitySystemComponent->TryActivateAbilityByClass(Ability);
		}
	}
}

void AKFBB_Ball::InitAbilities()
{
	for (auto AbilityToGrant : AbilityList)
	{
		GrantAbility(AbilityToGrant);
	}
}

void AKFBB_Ball::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	auto KFGS = Cast<AKFBB_GameState>(GetWorld()->GetGameState());
	if (KFGS)
	{
		KFGS->UnregisterBall(this);
	}
}

void AKFBB_Ball::Reset()
{
	Super::Reset();
}

// Called every frame
void AKFBB_Ball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RegisterWithField();
	if (IsFree())
	{
		AdjustBallToTileCenter(DeltaTime);
	}
	
	//debug
	DrawDebug();
}

void AKFBB_Ball::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKFBB_Ball, OwningPlayer);
}

void AKFBB_Ball::RegisterWithField()
{
	bOnGround = false;
	if (OwningPlayer != nullptr)
	{
		RegisterWithTile(OwningPlayer->GetCurrentTile());
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
		if (CurrentTile.IsValid()) { GetCurrentTile()->UnRegisterActor(this); }
		CurrentTile = Tile;
		if (CurrentTile.IsValid()) { GetCurrentTile()->RegisterActor(this); }
	}
}

FORCEINLINE AKFBB_Field* AKFBB_Ball::GetField() const
{
	return Field;
}

FORCEINLINE UKFBB_FieldTile* AKFBB_Ball::GetCurrentTile() const
{
	return Field ? Field->GetTileByInfo(CurrentTile) : nullptr;
}

void AKFBB_Ball::RegisterWithPlayer(AKFBB_PlayerPawn* P)
{
	if (!P || !P->AbilitySystemComponent || !AbilitySystemComponent) { return; }
	if (OwningPlayer) { return; }

	OwningPlayer = P;
	OwningPlayer->NotifyGainPossession(this);
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ApplyGameplayEffectToSelf(GE_BallState_Possessed.GetDefaultObject(), 1, FGameplayEffectContextHandle());
	}
}

void AKFBB_Ball::UnRegisterWithPlayer()
{
	if (OwningPlayer == nullptr || !OwningPlayer->AbilitySystemComponent) { return; }

	OwningPlayer->NotifyLostPossession(this);
	OwningPlayer = nullptr;
}

void AKFBB_Ball::FumbleBall(UKFBB_FieldTile* DestTile)
{
	UnRegisterWithPlayer();

	const FDetachmentTransformRules dtr(EDetachmentRule::KeepWorld, false);
	DetachFromActor(dtr);
	
	if (BallSMC != nullptr && CurrentTile.IsValid())
	{
		FVector ballVel = (DestTile->TileLocation - GetCurrentTile()->TileLocation).GetSafeNormal2D() * 100;
		ballVel.Z += 250;

		FVector ballAngVel = FMath::VRand() * (FMath::FRandRange(-10.f, 10.f));
		
		BallSMC->SetSimulatePhysics(true);
		BallSMC->SetPhysicsLinearVelocity(ballVel);
		BallSMC->SetPhysicsAngularVelocityInDegrees(ballAngVel);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ApplyGameplayEffectToSelf(GE_BallState_Fumble.GetDefaultObject(), 1, FGameplayEffectContextHandle());
	}
}

bool AKFBB_Ball::CanBePickedUp() const
{
	return (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(UTagLibrary::StatusBallCanPickup));
}

bool AKFBB_Ball::IsPossessed() const
{
	return (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(UTagLibrary::BallStatePossessed));
//	return (OwningPlayer != nullptr);
}

bool AKFBB_Ball::IsFumbled() const
{
	return (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(UTagLibrary::BallStateFumble));
}

bool AKFBB_Ball::IsFree() const
{
	return (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(UTagLibrary::BallStateFree));
}

void AKFBB_Ball::StopMovement()
{
	auto SMC = Cast<UStaticMeshComponent>(GetRootComponent());
	if (SMC != nullptr)
	{
		BallSMC->SetSimulatePhysics(false);
		SMC->SetPhysicsLinearVelocity(FVector::ZeroVector);
		SMC->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ApplyGameplayEffectToSelf(GE_BallState_Free.GetDefaultObject(), 1, FGameplayEffectContextHandle());
	}
}

void AKFBB_Ball::AdjustBallToTileCenter(float DeltaTime)
{
	if (CurrentTile.IsValid())
	{
		FVector BallLocation = GetActorLocation();
		FVector BallToTileCenter = (GetCurrentTile()->TileLocation - BallLocation);
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

void AKFBB_Ball::DrawDebug() const
{
	DrawDebugCurrentTile();

	if (CanBePickedUp())
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), FVector(10.f), FColor::White);
	}
	if (IsFumbled())
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), FVector(8.f), FColor::Black);
	}
	if (IsPossessed())
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), FVector(12.f), FColor::Green);
	}
}

void AKFBB_Ball::DrawDebugCurrentTile() const
{
	if (Field == nullptr || !CurrentTile.IsValid()) { return; }

	GetCurrentTile()->DrawDebugTile(FVector(0, 0, 2));
}
