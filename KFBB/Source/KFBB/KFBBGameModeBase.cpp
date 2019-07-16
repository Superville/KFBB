// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBBGameModeBase.h"

// Engine Includes
#include "UObject/ConstructorHelpers.h"
#include "Engine/LevelScriptActor.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameSession.h"
#include "Blueprint/UserWidget.h"

// KFBB Includes
#include "KFBB_Field.h"
#include "KFBB_PlayerPawn.h"
#include "KFBB_CoachPC.h"
#include "KFBB_Ball.h"
#include "Game/KFBB_TeamInfo.h"
#include "Game/KFBB_GameState.h"


AKFBBGameModeBase::AKFBBGameModeBase()
{
	PlayerControllerClass = AKFBB_CoachPC::StaticClass();
}

void AKFBBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass != nullptr)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (CurrentWidget != nullptr)
		{
			CurrentWidget->AddToViewport();
		}
	}

	AKFBB_Field::AssignFieldActor(this, Field);
	SpawnTeams();
	if (GetNetMode() != NM_DedicatedServer)
	{
		SpawnTeamPlayers();
	}
}

void AKFBBGameModeBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	AKFBB_Field::AssignFieldActor(this, Field);

	SpawnTeams();
	if (GetNetMode() != NM_DedicatedServer)
	{
		SpawnTeamPlayers();
	}
}

void AKFBBGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AKFBBGameModeBase::IsMatchInProgress() const
{
	FName StateName = GetMatchState();
	if (StateName == KFBBMatchState::InProgress)
	{
		return true;
	}

	return false;
}

bool AKFBBGameModeBase::ReadyToStartMatch_Implementation()
{
	if (NumPlayers < 1) { return false; }

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AKFBB_CoachPC* PC = Cast<AKFBB_CoachPC>(Iterator->Get());
		if (!PC) { continue; }
		if (!PC->IsReadyToStart()) { return false; }
	}
	return true;
}

bool AKFBBGameModeBase::ReadyToEndMatch_Implementation()
{
	if (bDebug_InfiniteMatch) { return false; }

	auto KFGS = Cast<AKFBB_GameState>(GameState);
	if (!KFGS) { return false; }
	return (KFGS->MatchDuration > 0.f) && (KFGS->MatchTimeRemaining <= 0.f);
}

void AKFBBGameModeBase::OnMatchStateSet()
{
	//test
	UE_LOG(LogTemp, Warning, TEXT("MATCH STATE %s"), *MatchState.ToString());

	FGameModeEvents::OnGameModeMatchStateSetEvent().Broadcast(MatchState);
	// Call change callbacks
	if (MatchState == KFBBMatchState::WaitingToStart)
	{
		HandleMatchIsWaitingToStart();
	}
	else if (MatchState == KFBBMatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == KFBBMatchState::InReplay)
	{
		HandleReplayHasStarted();
	}
	else if (MatchState == KFBBMatchState::WaitingPostMatch)
	{
		HandleMatchHasEnded();
	}
	else if (MatchState == KFBBMatchState::LeavingMap)
	{
		HandleLeavingMap();
	}
	else if (MatchState == KFBBMatchState::Aborted)
	{
		HandleMatchAborted();
	}
}

void AKFBBGameModeBase::HandleMatchIsWaitingToStart()
{
	SpawnTeamPlayers();
//test	SpawnBall(nullptr);

	if (GameSession != nullptr)
	{
		GameSession->HandleMatchIsWaitingToStart();
	}

	// Calls begin play on actors, unless we're about to transition to match start

	if (!ReadyToStartMatch())
	{
		GetWorldSettings()->NotifyBeginPlay();
	}
}

void AKFBBGameModeBase::StartMatch()
{
	if (HasMatchStarted())
	{
		// Already started
		return;
	}

	//Let the game session override the StartMatch function, in case it wants to wait for arbitration
	if (GameSession->HandleStartMatchRequest())
	{
		return;
	}

	SetMatchState(KFBBMatchState::InProgress);
}

void AKFBBGameModeBase::HandleMatchHasStarted()
{
	GameSession->HandleMatchHasStarted();

	// start human players first
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && (PlayerController->GetPawn() == nullptr) && PlayerCanRestart(PlayerController))
		{
			RestartPlayer(PlayerController);
		}
	}

	// Make sure level streaming is up to date before triggering NotifyMatchStarted
	GEngine->BlockTillLevelStreamingCompleted(GetWorld());

	// First fire BeginPlay, if we haven't already in waiting to start match
	GetWorldSettings()->NotifyBeginPlay();

	// Then fire off match started
	GetWorldSettings()->NotifyMatchStarted();

	if (IsHandlingReplays() && GetGameInstance() != nullptr)
	{
		GetGameInstance()->StartRecordingReplay(GetWorld()->GetMapName(), GetWorld()->GetMapName());
	}
}

void AKFBBGameModeBase::HandleReplayHasStarted()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(TimerHandle_ReplayTimer, this, &AKFBBGameModeBase::OnReplayTimerExpired, ReplayDuration, false);
}

void AKFBBGameModeBase::OnReplayTimerExpired()
{
	ResetLevel();
	SetMatchState(KFBBMatchState::InProgress);
}

void AKFBBGameModeBase::HandleMatchHasEnded()
{
	GameSession->HandleMatchHasEnded();

	if (IsHandlingReplays() && GetGameInstance() != nullptr)
	{
		GetGameInstance()->StopRecordingReplay();
	}
}

void AKFBBGameModeBase::HandleLeavingMap()
{
}

void AKFBBGameModeBase::HandleMatchAborted()
{
}

FString AKFBBGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal /*= TEXT("")*/)
{
	auto PC = Cast<AKFBB_CoachPC>(NewPlayerController);
	if (PC)
	{
		int32 InTeamID = -1;
		if (UGameplayStatics::HasOption(Options, TEXT("TeamID")))
		{
			InTeamID = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("TeamID")));

		}
		else if (UGameplayStatics::HasOption(OptionsString, TEXT("TeamID"))) // Backup so that ServerGameOptions in editor still work
		{
			InTeamID = FCString::Atoi(*UGameplayStatics::ParseOption(OptionsString, TEXT("TeamID")));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("No TeamID provided in Options (remember to but '?' at the start!) - Default to TeamID=0"));
			InTeamID = 0;
		}

		auto TeamInfo = GetTeamInfoByID(InTeamID);
		if (TeamInfo)
		{
			TeamInfo->RegisterCoach(PC);
		}
	}

	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void AKFBBGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	RestartPlayer(NewPlayer);
}

AActor* AKFBBGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	//test
	return Super::ChoosePlayerStart_Implementation(Player);
}

APawn* AKFBBGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	//test
	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

void AKFBBGameModeBase::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer) { return; }

	auto PC = Cast<AKFBB_CoachPC>(NewPlayer);
	if (!PC) { return; }

	Super::RestartPlayer(NewPlayer);

// 	auto TeamInfo = GetTeamByController(PC);
// 	if (!TeamInfo) { return; }
// 	TeamInfo->GrantPossession(PC);
}



void AKFBBGameModeBase::ResetLevel()
{
	UE_LOG(LogGameMode, Verbose, TEXT("Reset %s"), *GetName());

	// Reset ALL controllers first
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AController* Controller = Iterator->Get();
		APlayerController* PlayerController = Cast<APlayerController>(Controller);
		if (PlayerController)
		{
			PlayerController->ClientReset();
		}
		Controller->Reset();
	}

	// Reset all actors (except controllers, the GameMode, and any other actors specified by ShouldReset())
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* A = *It;
		if (A && !A->IsPendingKill() && A != this && !A->IsA<AController>() && ShouldReset(A))
		{
			A->Reset();
		}
	}

	// Reset the GameMode
	Reset();

	// Notify the level script that the level has been reset
	ALevelScriptActor* LevelScript = GetWorld()->GetLevelScriptActor();
	if (LevelScript)
	{
		LevelScript->LevelReset();
	}
}

void AKFBBGameModeBase::Reset()
{
	Super::Reset();
}

UClass* AKFBBGameModeBase::GetDefaultBallClass()
{
	return DefaultBallClass;
}

UClass* AKFBBGameModeBase::GetDefaultPlayerClass()
{
	return DefaultPlayerClass;
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

void AKFBBGameModeBase::ResolveCollision(AKFBB_PlayerPawn* PawnA, AKFBB_PlayerPawn* PawnB)
{
	// PawnA = player already occupying tile
	// PawnB = player moving onto the tile, PawnB->PreviousTile will give direction of movement

	auto KnockDirA = Field->GetTileDir(PawnB->GetPreviousTile(), PawnB->GetCurrentTile());
	auto KnockDirB = Field->GetTileDir(PawnB->GetCurrentTile(), PawnB->GetPreviousTile());
	
// 	KnockDirA.x = 0;
// 	KnockDirB.y = 0;

	PawnA->KnockDown(KnockDirA);
	PawnB->KnockDown(KnockDirB);
}

void AKFBBGameModeBase::SpawnTeams()
{
	if (!TeamInfoClass) { return; }

	for (int TeamID = 0; TeamID < NumTeams; TeamID++)
	{
		auto TeamInfo = GetWorld()->SpawnActor<AKFBB_TeamInfo>(TeamInfoClass);
		if (!TeamInfo) { continue; }
		
		Teams.Add(TeamInfo);
		TeamInfo->Init(this, TeamID, Field, OptionsString);
	}

	auto KFGS = Cast<AKFBB_GameState>(GameState);
	if (KFGS)
	{
		KFGS->InitTeamInfo(NumTeams);
	}
}

void AKFBBGameModeBase::SpawnTeamPlayers()
{
	for (int TeamID = 0; TeamID < Teams.Num(); TeamID++)
	{
		auto TeamInfo = Teams[TeamID];
		if (!TeamInfo) { continue; }
		TeamInfo->SpawnPlayers();
	}
}

UClass* AKFBBGameModeBase::GetPawnClassByID(int PawnID)
{
	//todo replace... look up class string by ID, then load class
	return DefaultPlayerClass;
}

AKFBB_TeamInfo* AKFBBGameModeBase::GetTeamInfoByID(uint8 teamID)
{
	if (!Teams.IsValidIndex(teamID)) { return nullptr; }
	return Teams[teamID];
}

void AKFBBGameModeBase::RegisterCoach(AKFBB_CoachPC* PC, uint8 teamID)
{
	if (!PC || !Teams.IsValidIndex(teamID)) { return; }
	Teams[teamID]->RegisterCoach(PC);
}

void AKFBBGameModeBase::UnregisterCoach(AKFBB_CoachPC* PC)
{
	if (!PC) { return; }
	uint8 teamID = PC->GetTeamID();
	if (!Teams.IsValidIndex(teamID)) { return; }
	Teams[teamID]->UnregisterCoach(PC);
}

void AKFBBGameModeBase::RegisterTeamMember(AKFBB_PlayerPawn* P, uint8 teamID)
{
	if (!P || !Teams.IsValidIndex(teamID)) { return; }
	Teams[teamID]->RegisterTeamMember(P);
}

void AKFBBGameModeBase::UnregisterTeamMember(AKFBB_PlayerPawn* P)
{
	if (!P) { return; }
	uint8 teamID = P->GetTeamID();
	if (!Teams.IsValidIndex(teamID)) { return; }
	Teams[teamID]->UnregisterTeamMember(P);
}
