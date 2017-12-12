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
		if (CooldownTimer <= 0.f)
		{
			OnCooldownTimerExpired();
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

void AKFBB_PlayerPawn::ClearCooldownTimer() 
{ 
	SetCooldownTimer(0.f); 
}

void AKFBB_PlayerPawn::OnCooldownTimerExpired()
{
	ClearCooldownTimer();

	if (Status == EKFBB_PlayerState::Down)
	{
		SetStatus_StandUp();
	}
	else if (Status == EKFBB_PlayerState::Stunned)
	{
		SetStatus_KnockedDown();
	}
	else if (Status == EKFBB_PlayerState::Standing || 
			 Status == EKFBB_PlayerState::Exhausted)
	{
		SetStatus_Ready();
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
			SetStatus_Moving();
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
		SetStatus_Exhausted();
	}	
}

void AKFBB_PlayerPawn::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	AKFBB_PlayerPawn* OtherPlayerPawn = Cast<AKFBB_PlayerPawn>(Other);
	if (OtherPlayerPawn != nullptr)
	{
		SetStatus_KnockedDown();
		OtherPlayerPawn->SetStatus_KnockedDown();
	}
}

void AKFBB_PlayerPawn::SetStatus_Moving()
{
	Status = EKFBB_PlayerState::Moving;

	//test
	UE_LOG(LogTemp, Warning, TEXT("%s Moving"), *GetName());
}

void AKFBB_PlayerPawn::SetStatus_Exhausted()
{
	Status = EKFBB_PlayerState::Exhausted;
	SetCooldownTimer(ExhaustedCooldownTime);

	//test
	UE_LOG(LogTemp, Warning, TEXT("%s Exhausted"), *GetName());
}

void AKFBB_PlayerPawn::SetStatus_KnockedDown()
{
	AAIController* ai = Cast<AAIController>(Controller);
	ai->StopMovement();
	MoveToTileLocation(CurrentTile);

	Status = EKFBB_PlayerState::Down;
	SetCooldownTimer(KnockedDownTime);

	//test
	UE_LOG(LogTemp, Warning, TEXT("%s Knocked Down"), *GetName());
}

void AKFBB_PlayerPawn::SetStatus_Stunned()
{
	Status = EKFBB_PlayerState::Stunned;
	SetCooldownTimer(StunnedTime);

	//test
	UE_LOG(LogTemp, Warning, TEXT("%s Stunned!"), *GetName());
}

void AKFBB_PlayerPawn::SetStatus_StandUp()
{
	Status = EKFBB_PlayerState::Standing;
	SetCooldownTimer(StandUpTime);

	//test
	UE_LOG(LogTemp, Warning, TEXT("%s Standup!"), *GetName());
}

void AKFBB_PlayerPawn::SetStatus_Ready()
{
	Status = EKFBB_PlayerState::Normal;
	
	//test
	UE_LOG(LogTemp, Warning, TEXT("%s Ready!"), *GetName());
}

void AKFBB_PlayerPawn::DrawDebugCurrentTile() const
{
	if (Field == nullptr || CurrentTile == nullptr)
		return;

	CurrentTile->DrawDebugTile(FVector(0, 0, 2));
}

void AKFBB_PlayerPawn::DrawDebugStatus() const
{
	bool bDraw = (Status != EKFBB_PlayerState::Normal);

	if (bDraw)
	{
		auto MyWorld = GetWorld();
		FColor color = GetDebugColor();
		DrawDebugBox(MyWorld, GetActorLocation(), FVector(20, 20, 20), color, false, 1.f);
	}
}
FColor AKFBB_PlayerPawn::GetDebugColor() const
{
	AAIController* ai = Cast<AAIController>(Controller);
	if (Status == EKFBB_PlayerState::Down) { return FColor::Black; }
	else if (Status == EKFBB_PlayerState::Stunned) { return FColor::Blue;	}
	else if (Status == EKFBB_PlayerState::Standing) { return FColor::Yellow; }
	else if (Status == EKFBB_PlayerState::Moving) { return FColor::Green; }
	else if (Status == EKFBB_PlayerState::Exhausted) { return FColor::Orange; }
	return FColor::Red;
}