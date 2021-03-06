// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_PlayerPawn.h"

// Engine Includes
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "AbilitySystemComponent.h"

// KFBB Includes
#include "KFBB_GameplayAbility.h"
#include "KFBB_AIController.h"
#include "KFBB_CoachPC.h"
#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"
#include "KFBB_Ball.h"
#include "KFBB.h"
#include "KFBB_BallMovementComponent.h"
#include "KFBBGameModeBase.h"
#include "Player/KFBBAttributeSet.h"
#include "Game/KFBB_TeamInfo.h"

// Sets default values
AKFBB_PlayerPawn::AKFBB_PlayerPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Pill = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pill"));
	Pill->SetupAttachment(RootComponent);
	Pill->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.9f));
	Pill->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Pill->SetMobility(EComponentMobility::Movable);
	Pill->SetGenerateOverlapEvents(false);


	AttribSet = CreateDefaultSubobject<UKFBBAttributeSet>(FName("AttribSet"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(FName("AbilitySystemComp"));
}

UAbilitySystemComponent* AKFBB_PlayerPawn::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AKFBB_PlayerPawn::GrantAbility(TSubclassOf<UKFBB_GameplayAbility> Ability)
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

void AKFBB_PlayerPawn::InitAbilities()
{
	for (auto AbilityToGrant : AbilityList)
	{
		GrantAbility(AbilityToGrant);
	}
}

int32 AKFBB_PlayerPawn::GetStat_Movement()
{
	return AttribSet ? AttribSet->Stat_Movement.GetCurrentValue() : 0;
}

// Called when the game starts or when spawned
void AKFBB_PlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
	AKFBB_Field::AssignFieldActor(this, Field);
	RegisterWithField();

	InitAbilities();

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
		if (CurrentTile.IsValid())
		{
			GetCurrentTile()->DrawCooldownTimer(CooldownDuration, CooldownTimer, GetCooldownColor());
		}

		if (CooldownTimer <= 0.f)
		{
			OnCooldownTimerExpired();
		}
	}
	
	//debug
	DrawDebug();
}

// Called to bind functionality to input
void AKFBB_PlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AKFBB_PlayerPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKFBB_PlayerPawn, TeamID);

	DOREPLIFETIME(AKFBB_PlayerPawn, DestinationTile);
	DOREPLIFETIME(AKFBB_PlayerPawn, PathToDestTile);
}

bool AKFBB_PlayerPawn::IsMyCoach(AKFBB_CoachPC* inCoach) const
{
	auto KFGM = Cast<AKFBBGameModeBase>(GetWorld()->GetAuthGameMode());
	if (KFGM)
	{
		auto TeamInfo = KFGM->GetTeamInfoByID(GetTeamID());
		if (TeamInfo)
		{
			return TeamInfo->IsCoach(inCoach);
		}		
	}

	return false;
}

bool AKFBB_PlayerPawn::IsSameTeam(AKFBB_PlayerPawn* Other) const
{
	return Other ? GetTeamID() == Other->GetTeamID() : false;
}

AKFBB_Field* AKFBB_PlayerPawn::GetField() const
{
	return Field;
}

UKFBB_FieldTile* AKFBB_PlayerPawn::GetCurrentTile() const
{
	return Field ? Field->GetTileByInfo(CurrentTile) : nullptr;
}

UKFBB_FieldTile* AKFBB_PlayerPawn::GetPreviousTile() const
{
	return Field ? Field->GetTileByInfo(PreviousTile) : nullptr;
}

UKFBB_FieldTile* AKFBB_PlayerPawn::GetNextTile() const
{
	return Field ? Field->GetTileByInfo(NextTile) : nullptr;
}

bool AKFBB_PlayerPawn::CanBeSelected(AKFBB_CoachPC* inCoach) const
{
	if (!IsMyCoach(inCoach)) { return false; }
	if (Status == EKFBB_PlayerState::Moving) { return false; }
	return true;
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

void AKFBB_PlayerPawn::RegisterWithTile(UKFBB_FieldTile* Tile)
{
	if (CurrentTile != Tile)
	{
		auto PlayerAlreadyOnTile = Tile ? Tile->GetPlayer() : nullptr;
		if (CurrentTile.IsValid())
		{ 
			GetCurrentTile()->UnRegisterActor(this); 
		}
		
		PreviousTile = CurrentTile;
		CurrentTile = Tile;
		
		if (CurrentTile.IsValid()) 
		{ 
			GetCurrentTile()->RegisterActor(this); 
		}

		//todo - convert this to using GAS / tags
		if (PlayerAlreadyOnTile != nullptr)
		{
			auto GM = Cast<AKFBBGameModeBase>(GetWorld()->GetAuthGameMode());
			if (GM)
			{
				GM->ResolveCollision(PlayerAlreadyOnTile, this);
			}
		}
	}
}

void AKFBB_PlayerPawn::SetCooldownTimer(float t) 
{ 
	CooldownDuration = t;
	CooldownTimer = t; 
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

FColor AKFBB_PlayerPawn::GetCooldownColor() const
{
	switch (Status)
	{
	case EKFBB_PlayerState::GrabBall:
		return FColor::White;
		break;
	case EKFBB_PlayerState::KnockedDown:
		return FColor::Blue;
		break;
	case EKFBB_PlayerState::Stunned:
		return FColor::Black;
		break;
	case EKFBB_PlayerState::StandingUp:
		return FColor::Yellow;
		break;
	case EKFBB_PlayerState::Exhausted:
		return FColor::Orange;
		break;
	case EKFBB_PlayerState::Moving:
		return FColor::Green;
		break;
	default:
		break;
	}

	return FColor::Transparent;
}


bool AKFBB_PlayerPawn::CanAcceptCommand()
{
	if (IsPlayerOnCooldown()) { return false; }
	if (AbilitySystemComponent)
	{
		if (AbilitySystemComponent->GetTagCount(ProhibitCommandsTag) > 0) { return false; }
	}

	return true;
}

void AKFBB_PlayerPawn::NotifyCommandFailed()
{
	auto MyWorld = GetWorld();
	FColor color = FColor::Red;
	DrawDebugBox(MyWorld, GetActorLocation(), FVector(20, 20, 20), color, false, 1.f);
}

void AKFBB_PlayerPawn::NotifyReachedGrid()
{
	if (!CurrentTile.IsValid()) { return; }

	auto BallOnTile = GetCurrentTile()->GetBall();
	if (BallOnTile && AbilitySystemComponent)
	{
		if (AbilitySystemComponent->GetTagCount(UTagLibrary::StatusPlayerHasBall) == 0 &&
			BallOnTile->CanBePickedUp())
		{
			AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(TriggerPickupAbilityTag));
			auto AI = Cast<AKFBB_AIController>(GetController());
			if (AI)
			{
				AI->ClearDestination();
			}
		}
	}
}

void AKFBB_PlayerPawn::NotifyReachedDestinationGrid()
{
	auto AI = Cast<AKFBB_AIController>(GetController());
	if (!AI) { return; }

	if (Status == EKFBB_PlayerState::Moving)
	{
		SetStatus(EKFBB_PlayerState::Exhausted);
	}

	AI->ClearDestination();
}

UKFBB_FieldTile* AKFBB_PlayerPawn::GetFirstGrid() const
{
	if (!Field || PathToDestTile.Num() <= 0) { return nullptr; }
	return Field->GetTileByInfo(PathToDestTile[0]);
}

void AKFBB_PlayerPawn::RemoveFirstGrid()
{
	if (PathToDestTile.Num() <= 0) { return; }
	PathToDestTile.RemoveAt(0);
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

void AKFBB_PlayerPawn::KnockDown(FTileDir dir)
{
	auto AI = Cast<AKFBB_AIController>(Controller);
	if (AI != nullptr)
	{
//		AI->MarkDestinationTile(Field->GetAdjacentTile(CurrentTile, dir));
	}

	SetStatus(EKFBB_PlayerState::KnockedDown);
}

void AKFBB_PlayerPawn::SetStatus(EKFBB_PlayerState::Type newStatus)
{
	Status = newStatus;

	auto AI = Cast<AKFBB_AIController>(Controller);
	
	switch (Status)
	{
	case EKFBB_PlayerState::Moving:
		break;
	case EKFBB_PlayerState::Exhausted:
		SetCooldownTimer(AttribSet->Stat_ExhaustCD.GetCurrentValue());
		break;
	case EKFBB_PlayerState::KnockedDown:
		SetCooldownTimer(AttribSet->Stat_KnockDownCD.GetCurrentValue());
		break;
	case EKFBB_PlayerState::Stunned:
		SetCooldownTimer(AttribSet->Stat_StunCD.GetCurrentValue());
		break;
	case EKFBB_PlayerState::StandingUp:
		SetCooldownTimer(AttribSet->Stat_StandCD.GetCurrentValue());
		break;
	case EKFBB_PlayerState::Ready:
		break;
	case EKFBB_PlayerState::GrabBall:
		AI->ClearDestination();
		break;
	}
	
//	UE_LOG(LogTemp, Warning, TEXT("%s SetStatus %s"), *GetName(), *GetStatusString());
}

bool AKFBB_PlayerPawn::HasBall() const
{
	return (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(UTagLibrary::StatusPlayerHasBall));
//	return (Ball != nullptr);
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
	//test
//	return true;
//	return false;

	auto roll = FMath::RandRange(1, 6);
	int chance = 4;

	UE_LOG(LogTemp, Warning, TEXT("%s Try Pickup BallOnTile Chance %d Roll %d - %s"), *GetName(), chance, roll, (roll >= chance) ? TEXT("Success!") : TEXT("Fumble!"));
	
	return (roll >= chance);
}

void AKFBB_PlayerPawn::ClaimBall(AKFBB_Ball* BallToClaim)
{
	if (!BallToClaim || !AbilitySystemComponent) { return; }

	Ball = BallToClaim;
	Ball->RegisterWithPlayer(this);

	AttachBall(BallToClaim);
}

void AKFBB_PlayerPawn::AttachBall(AKFBB_Ball* BallToAttach)
{
	if (BallToAttach != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s AttachBall %s"), *GetName(), *BallToAttach->GetName());

		BallToAttach->BallSMC->SetSimulatePhysics(false);
		BallToAttach->AttachToComponent(Pill, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("BallSocket"));
	}
}


void AKFBB_PlayerPawn::FumbleBall(AKFBB_Ball* BallToFumble)
{
	if (!BallToFumble || 
		!CurrentTile.IsValid() || 
		!Field || 
		!AbilitySystemComponent)
	{
		return;
	}

	auto destTile = Field->GetAdjacentTile(GetCurrentTile(), Field->GetScatterDirection(0, 0, 4));
	BallToFumble->FumbleBall(destTile);
}

void AKFBB_PlayerPawn::NotifyGainPossession(AKFBB_Ball* ball)
{
	Ball = ball;
	if (AbilitySystemComponent)
	{
		Handle_StatusHasBall = AbilitySystemComponent->ApplyGameplayEffectToSelf(GE_StatusHasBall.GetDefaultObject(), 1, FGameplayEffectContextHandle());
	}
}

void AKFBB_PlayerPawn::NotifyLostPossession(AKFBB_Ball* ball)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(Handle_StatusHasBall);
	}
	Ball = nullptr;	
}

UKFBB_FieldTile* AKFBB_PlayerPawn::GetDestinationTile() const
{
	return Field ? Field->GetTileByInfo(DestinationTile) : nullptr;
}

void AKFBB_PlayerPawn::SetDestinationTile(FTileInfo TileRepInfo)
{
	DestinationTile = TileRepInfo;
}

void AKFBB_PlayerPawn::ClearDestinationTile()
{
	DestinationTile.Clear();
}

bool AKFBB_PlayerPawn::HasPathToDestTile()
{
	return (PathToDestTile.Num() > 0);
}

void AKFBB_PlayerPawn::SetPathToDestTile(TArray<FTileInfo>& InPathToDest)
{
	PathToDestTile = InPathToDest;
}

void AKFBB_PlayerPawn::ClearPathToDestTile()
{
	PathToDestTile.Empty();
}

void AKFBB_PlayerPawn::DrawDebug() const
{
//	DrawDebugCurrentTile();
	DrawDebugStatus();
	DrawDebugPath();
}

void AKFBB_PlayerPawn::DrawDebugCurrentTile() const
{
	if (Field == nullptr || !CurrentTile.IsValid()) { return; }

	GetCurrentTile()->DrawDebugTile(FVector(0, 0, 2));
}

void AKFBB_PlayerPawn::DrawDebugStatus() const
{
	if (HasBall())
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), FVector(20, 20, 40), FColor::Green);
	}



	/*bool bDraw = (Status != EKFBB_PlayerState::Ready);

	if (bDraw)
	{
		auto MyWorld = GetWorld();
		FColor color = GetDebugColor();
		DrawDebugBox(MyWorld, GetActorLocation(), FVector(20, 20, 20), color, false);
	}*/
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

	AttribSet->Stat_Movement.SetBaseValue(data.MA);
	AttribSet->Stat_Strength.SetBaseValue(data.ST);
	AttribSet->Stat_Agility.SetBaseValue(data.AG);
	AttribSet->Stat_Armor.SetBaseValue(data.AV);
	
	AttribSet->Stat_ExhaustCD.SetBaseValue(data.ExhaustCD);
	AttribSet->Stat_KnockDownCD.SetBaseValue(data.KnockCD);
	AttribSet->Stat_StunCD.SetBaseValue(data.StunCD);
	AttribSet->Stat_StandCD.SetBaseValue(data.StandCD);
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

uint8 AKFBB_PlayerPawn::GetTeamID() const
{
	return TeamID;
}

void AKFBB_PlayerPawn::SetTeamID(uint8 teamID)
{
	TeamID = teamID;
	OnRep_TeamID();
}

void AKFBB_PlayerPawn::OnRep_TeamID()
{
	auto LocalCoach = Cast<AKFBB_CoachPC>(GetWorld()->GetFirstPlayerController());
	if (LocalCoach)
	{
		DisplayTeamID = (LocalCoach->GetTeamID() == GetTeamID()) ? 0 : 1;
		BP_UpdateTeamColor();
	}
}

uint8 AKFBB_PlayerPawn::GetDisplayTeamID() const
{
	return DisplayTeamID;
}

void AKFBB_PlayerPawn::DrawDebugPath() const
{
	FVector offset(0, 0, 2);
	UKFBB_FieldTile* LastTile = nullptr;

	for (int i = 0; i < PathToDestTile.Num(); i++)
	{
		auto Tile = Field->GetTileByInfo(PathToDestTile[i]);
		if (!Tile) { continue; }
		Tile->DrawDebugTileOverride(offset, 0.25f, FColor::Emerald);
		if (LastTile != nullptr)
		{
			DrawDebugLine(GetWorld(), LastTile->TileLocation + offset, Tile->TileLocation + offset, FColor::Emerald);
		}
		else
		{
			FVector PawnLoc = GetActorLocation();
			PawnLoc.Z = Tile->TileLocation.Z;
			DrawDebugLine(GetWorld(), PawnLoc + offset, Tile->TileLocation + offset, FColor::Emerald);
		}
		LastTile = Tile;
	}
}