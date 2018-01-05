// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_PlayerPawn.h"
#include "KFBB_AIController.h"
#include "KFBB_CoachPC.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "KFBB_Ball.h"
#include "KFBB.h"
#include "DrawDebugHelpers.h"
#include "KFBB_BallMovementComponent.h"

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
		if (CooldownTimer <= 0.f)
		{
			OnCooldownTimerExpired();
		}
	}

	if (CurrentTile != nullptr && CurrentTile->HasBall())
	{
		auto ball = CurrentTile->GetBall();
		if (CanPickupBall(ball) && TryPickupBall())
		{
			ClaimBall();
		}
		else
		{
			FumbleBall();
		}
	}

	//debug
	DrawDebugCurrentTile();
	DrawDebugStatus();
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
	if (MyWorld->LineTraceSingleByChannel(Hit, Loc, Loc + FVector(0,0,-1000), ECollisionChannel::ECC_FieldTrace))
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

void AKFBB_PlayerPawn::ClearCooldownTimer() 
{ 
	SetCooldownTimer(0.f); 
}

void AKFBB_PlayerPawn::OnCooldownTimerExpired()
{
	ClearCooldownTimer();

	if (Status == EKFBB_PlayerState::KnockedDown)
	{
		SetStatus(EKFBB_PlayerState::StandingUp);
	}
	else if (Status == EKFBB_PlayerState::Stunned)
	{
		SetStatus(EKFBB_PlayerState::KnockedDown);
	}
	else if (Status == EKFBB_PlayerState::StandingUp || 
			 Status == EKFBB_PlayerState::Exhausted)
	{
		SetStatus(EKFBB_PlayerState::Ready);
	}
}
bool AKFBB_PlayerPawn::IsPlayerOnCooldown() 
{ 
	return (CooldownTimer > 0.f); 
}


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

bool AKFBB_PlayerPawn::MoveToTileLocation(UKFBB_FieldTile* Tile)
{
	if (Tile == nullptr)
		return false;

	AAIController* ai = Cast<AAIController>(Controller);
	if (ai != nullptr)
	{
		FVector Dest = Tile->TileLocation;
		auto result = ai->MoveToLocation(Dest, -1.f, false, false, false, false);
		if (result == EPathFollowingRequestResult::RequestSuccessful)
		{
			return true;
		}
	}
	return false;
}

bool AKFBB_PlayerPawn::NotifyCommandGiven(UKFBB_FieldTile* DestinationTile)
{
	if (CanAcceptCommand() && Controller != nullptr)
	{
		if (MoveToTileLocation(DestinationTile))
		{
			SetStatus(EKFBB_PlayerState::Moving);
			return true;
		}
		else
		{
			NotifyCommandFailed();
			return false;
		}
	}
	return false;
}

void AKFBB_PlayerPawn::NotifyCommandFailed()
{
	auto MyWorld = GetWorld();
	FColor color = FColor::Red;
	DrawDebugBox(MyWorld, GetActorLocation(), FVector(20, 20, 20), color, false, 1.f);
}

void AKFBB_PlayerPawn::NotifyReachedDestination()
{
	if( Status == EKFBB_PlayerState::Moving)
	{
		SetStatus(EKFBB_PlayerState::Exhausted);
	}	
}

void AKFBB_PlayerPawn::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	AKFBB_PlayerPawn* OtherPlayerPawn = Cast<AKFBB_PlayerPawn>(Other);
	if (OtherPlayerPawn != nullptr)
	{
		SetStatus(EKFBB_PlayerState::KnockedDown);
		OtherPlayerPawn->SetStatus(EKFBB_PlayerState::KnockedDown);
	}
}

void AKFBB_PlayerPawn::SetStatus(EKFBB_PlayerState::Type newStatus)
{
	Status = newStatus;
	TestStatus = Status;

	AAIController* ai = Cast<AAIController>(Controller);
	
	switch (Status)
	{
	case EKFBB_PlayerState::Moving:
		break;
	case EKFBB_PlayerState::Exhausted:
		SetCooldownTimer(ExhaustedCooldownTime);
		break;
	case EKFBB_PlayerState::KnockedDown:
		ai->StopMovement();
		if (CurrentTile != nullptr)
		{
			MoveToTileLocation(CurrentTile);
		}
		SetCooldownTimer(KnockedDownTime);
		break;
	case EKFBB_PlayerState::Stunned:
		SetCooldownTimer(StunnedTime);
		break;
	case EKFBB_PlayerState::StandingUp:
		SetCooldownTimer(StandUpTime);
		break;
	case EKFBB_PlayerState::Ready:
		break;
	case EKFBB_PlayerState::GrabBall:
		ai->StopMovement();
		if (CurrentTile != nullptr)
		{
			MoveToTileLocation(CurrentTile);
		}
		break;
	}
	
//	UE_LOG(LogTemp, Warning, TEXT("%s SetStatus %s"), *GetName(), *GetStatusString());
}

bool AKFBB_PlayerPawn::HasBall() const
{
	return (Ball != nullptr);
}

bool AKFBB_PlayerPawn::CanPickupBall(AKFBB_Ball* ball) const
{
	if (Status == EKFBB_PlayerState::Ready || 
		Status == EKFBB_PlayerState::Exhausted ||
		Status == EKFBB_PlayerState::Moving)
	{
		if (HasBall() == false && ball->CanBePickedUp())
		{
			return true;
		}
	}

	return false;
}

bool AKFBB_PlayerPawn::TryPickupBall() const
{
	auto roll = FMath::RandRange(1, 6);
	int chance = 4;

	//test
	UE_LOG(LogTemp, Warning, TEXT("%s Try Pickup Ball Chance %d Roll %d - %s"), *GetName(), chance, roll, (roll >= chance) ? TEXT("Success!") : TEXT("Fumble!"));
	
	return (roll >= chance);
}

void AKFBB_PlayerPawn::ClaimBall()
{
	if (CurrentTile == nullptr || CurrentTile->HasBall() == false)
	{
		//error!
		return;
	}

	Ball = CurrentTile->GetBall();
	if (Ball != nullptr)
	{
		Ball->RegisterWithPlayer(this);
		SetStatus(EKFBB_PlayerState::GrabBall);
	}
}

void AKFBB_PlayerPawn::AttachBall()
{
	if (Ball != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s AttachBall %s"), *GetName(), *Ball->GetName());

		const FAttachmentTransformRules atr(EAttachmentRule::KeepRelative,true);
		Ball->AttachToActor(this, atr, FName("BallSocket"));
		Ball->SetActorRelativeLocation(FVector::ZeroVector);
		Ball->SetActorRelativeRotation(FRotator::ZeroRotator);

		SetStatus(EKFBB_PlayerState::Ready);
	}
}


void AKFBB_PlayerPawn::FumbleBall()
{
	auto b = CurrentTile->GetBall();
	auto v = b->GetVelocity();
	if (b->IsMoving())
	{
		return;
	}

	if (Ball != nullptr)
	{
		const FDetachmentTransformRules dtr(EDetachmentRule::KeepWorld, false);
		Ball->DetachFromActor(dtr);
		Ball = nullptr;
	}
	auto destTile = Field->GetAdjacentTile(CurrentTile, Field->GetScatterDirection(0, 0, 4));

//	auto MyWorld = GetWorld();
//	UGameplayStatics::SuggestProjectileVelocity(MyWorld,)

	b->FumbleBall(destTile);
}

void AKFBB_PlayerPawn::DrawDebugCurrentTile() const
{
	if (Field == nullptr || CurrentTile == nullptr)
		return;

	CurrentTile->DrawDebugTile(FVector(0, 0, 2));
}

void AKFBB_PlayerPawn::DrawDebugStatus() const
{
	bool bDraw = (Status != EKFBB_PlayerState::Ready);

	if (bDraw)
	{
		auto MyWorld = GetWorld();
		FColor color = GetDebugColor();
		DrawDebugBox(MyWorld, GetActorLocation(), FVector(20, 20, 20), color, false);
	}
}
FColor AKFBB_PlayerPawn::GetDebugColor() const
{
	AAIController* ai = Cast<AAIController>(Controller);
	if (Status == EKFBB_PlayerState::KnockedDown) { return FColor::Black; }
	else if (Status == EKFBB_PlayerState::Stunned) { return FColor::Blue;	}
	else if (Status == EKFBB_PlayerState::StandingUp) { return FColor::Yellow; }
	else if (Status == EKFBB_PlayerState::Moving) { return FColor::Green; }
	else if (Status == EKFBB_PlayerState::Exhausted) { return FColor::Orange; }
	return FColor::Red;
}

FString AKFBB_PlayerPawn::GetStatusString() const
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EKFBB_PlayerState"), true);
	if (!EnumPtr) return FString("Invalid");

	return EnumPtr->GetNameStringByValue((int64)Status);
}

void AKFBB_PlayerPawn::LoadAttributesFromPlayerData(const FKFBB_PlayerData& data)
{
	Race = data.Race;
	Position = data.Position;
	Cost = data.Cost;
	MA = data.MA;
	ST = data.ST;
	AG = data.AG;
	AV = data.AV;
	
	ExhaustedCooldownTime = data.ExhaustCD;
	KnockedDownTime = data.KnockCD;
	StunnedTime = data.StunCD;
	StandUpTime = data.StandCD;
}

void AKFBB_PlayerPawn::LoadAttributesByDataName(FString RowName)
{
	if (PlayerDataTable)
	{
		static const FString ContextString(TEXT("GENERAL"));
		FKFBB_PlayerData* r = PlayerDataTable->FindRow<FKFBB_PlayerData>(FName(*RowName),ContextString);
		if (r != nullptr)
		{
			LoadAttributesFromPlayerData(*r);
		}
	}
}
