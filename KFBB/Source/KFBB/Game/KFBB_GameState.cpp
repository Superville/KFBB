// Fill out your copyright notice in the Description page of Project Settings.


#include "KFBB_GameState.h"

// Engine Includes
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Classes/Kismet/GameplayStatics.h"

// KFBB Includes
#include "KFBBGameModeBase.h"
#include "KFBB_Ball.h"
#include "KFBB_PlayerPawn.h"


AKFBB_GameState::AKFBB_GameState()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	InitGameplayTags();
}

void AKFBB_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKFBB_GameState, MatchDuration);
	DOREPLIFETIME(AKFBB_GameState, TeamScores);
}

void AKFBB_GameState::BeginPlay()
{
	Super::BeginPlay();

	MatchTimeRemaining = MatchDuration;
	MatchTimeRemainingString = FormatTimeString(MatchTimeRemaining);
}

void AKFBB_GameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FName StateName = GetMatchState();
	if (StateName == KFBBMatchState::InProgress)
	{
		MatchTimeRemaining = FMath::Max(MatchTimeRemaining - DeltaTime, 0.f);
	}
	else if (StateName == KFBBMatchState::WaitingPostMatch)
	{
		MatchTimeRemaining = 0.f;
	}
	MatchTimeRemainingString = FormatTimeString(MatchTimeRemaining);
}

void AKFBB_GameState::InitGameplayTags()
{
	//test
	//todo - store gameplay global tags here?
}

bool AKFBB_GameState::IsMatchInProgress() const
{
	FName StateName = GetMatchState();
	if (StateName == KFBBMatchState::InProgress)
	{
		return true;
	}

	return false;
}

void AKFBB_GameState::OnRep_MatchState()
{
	if (MatchState == KFBBMatchState::WaitingToStart || PreviousMatchState == KFBBMatchState::EnteringMap)
	{
		// Call MatchIsWaiting to start even if you join in progress at a later state
		HandleMatchIsWaitingToStart();
	}

	if (MatchState == KFBBMatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == KFBBMatchState::InReplay)
	{
		HandleMatchReplayStarted();
	}
	else if (MatchState == KFBBMatchState::WaitingPostMatch)
	{
		HandleMatchHasEnded();
	}
	else if (MatchState == KFBBMatchState::LeavingMap)
	{
		HandleLeavingMap();
	}

	PreviousMatchState = MatchState;
}

void AKFBB_GameState::HandleMatchHasStarted()
{
	if (Role != ROLE_Authority)
	{
		// Server handles this in AGameMode::HandleMatchHasStarted
		GetWorldSettings()->NotifyMatchStarted();
	}
	else
	{
		// Now that match has started, act like the base class and set replicated flag
		bReplicatedHasBegunPlay = true;
	}

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.UnPauseTimer(TimerHandle_DefaultTimer);
}

void AKFBB_GameState::HandleMatchReplayStarted()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.PauseTimer(TimerHandle_DefaultTimer);
}

void AKFBB_GameState::RegisterPlayer(AKFBB_PlayerPawn* Player)
{
	PlayerList.AddUnique(Player);
}

void AKFBB_GameState::UnregisterPlayer(AKFBB_PlayerPawn* Player)
{
	PlayerList.Remove(Player);
}

void AKFBB_GameState::RegisterBall(AKFBB_Ball* Ball)
{
	BallList.AddUnique(Ball);
}

void AKFBB_GameState::UnregisterBall(AKFBB_Ball* Ball)
{
	BallList.Remove(Ball);
}

AKFBB_Ball* AKFBB_GameState::GetBall(int32 idx /*= 0*/) const
{
	if (BallList.IsValidIndex(idx))
	{
		return BallList[idx];
	}
	return nullptr;
}

FString AKFBB_GameState::FormatTimeString(float InSeconds)
{
	// Get whole minutes
	const int32 NumMinutes = FMath::FloorToInt(InSeconds / 60.f);
	// Get seconds not part of whole minutes
	const int32 NumSeconds = FMath::FloorToInt(InSeconds - (NumMinutes*60.f));
	// Get fraction of non-whole seconds, convert to 100th of a second, then floor to get whole 100ths
	const int32 NumCentiseconds = FMath::FloorToInt((InSeconds - FMath::FloorToFloat(InSeconds)) * 10.f);

	if (NumMinutes == 0)
	{
		if (NumSeconds < 10)
		{
			return FString::Printf(TEXT("%01d.%01d"), NumSeconds, NumCentiseconds);
		}
		return FString::Printf(TEXT("0:%02d"), NumSeconds);
	}
	else if (NumMinutes < 10)
	{
		return FString::Printf(TEXT("%01d:%02d"), NumMinutes, NumSeconds);
	}
	else
	{
		return FString::Printf(TEXT("%02d:%02d"), NumMinutes, NumSeconds);
	}
}

void AKFBB_GameState::InitTeamInfo(int32 TeamNum)
{
	TeamScores.AddZeroed(TeamNum);
}

int32 AKFBB_GameState::GetScoreByTeamID(uint8 TeamID)
{
	if (!TeamScores.IsValidIndex(TeamID)) { return -1; }
	return TeamScores[TeamID];
}

void AKFBB_GameState::OnRep_TeamScore()
{
	if (TeamScores[0] > 0 || TeamScores[1] > 0)
	{
		if (GoalParticleSystem)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GoalParticleSystem, GetBall()->GetActorLocation());
		}
	}
}

void AKFBB_GameState::ScoreGoalFor(int32 TeamID, int32 ScoreAdd)
{
	if (!TeamScores.IsValidIndex(TeamID)) { return; }

	TeamScores[TeamID] += ScoreAdd;
	if (GetNetMode() != NM_DedicatedServer)
	{
		OnRep_TeamScore();
	}

	UE_LOG(LogTemp, Error, TEXT("SCORE GOAL FOR Team %d -- Score: %d"), TeamID, TeamScores[TeamID]);
}