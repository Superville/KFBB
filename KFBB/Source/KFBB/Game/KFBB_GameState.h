// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "KFBB_GameState.generated.h"

class UParticleSystem;
class AKFBB_PlayerPawn;
class AKFBB_Ball;

/**
 * 
 */
UCLASS()
class KFBB_API AKFBB_GameState : public AGameState
{
	GENERATED_BODY()
	
public:
	AKFBB_GameState();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Match State
	virtual bool IsMatchInProgress() const override;
	virtual void OnRep_MatchState();
	virtual void HandleMatchHasStarted();
	virtual void HandleMatchReplayStarted();

	// Register / Getters
	UPROPERTY(VisibleAnywhere, Category = "Player")
	TArray<AKFBB_PlayerPawn*> PlayerList;
	virtual void RegisterPlayer(AKFBB_PlayerPawn* Player);
	virtual void UnregisterPlayer(AKFBB_PlayerPawn* Player);

	UPROPERTY(VisibleAnywhere, Category = "Ball")
	TArray<AKFBB_Ball*> BallList;
	virtual void RegisterBall(AKFBB_Ball* Ball);
	virtual void UnregisterBall(AKFBB_Ball* Ball);
	UFUNCTION(BlueprintCallable, Category = "Balls")
	AKFBB_Ball* GetBall(int32 idx = 0) const;


	// Match Duration
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Timers")
	float MatchDuration = 180.f;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Timers")
	float MatchTimeRemaining;
	UPROPERTY(BlueprintReadOnly, Category = "Timers")
	FString MatchTimeRemainingString;
	FString FormatTimeString(float InSeconds);

	// Team Info
	virtual void InitTeamInfo(int32 TeamNum);

protected:
	// Score / Goals
	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing = OnRep_TeamScore)
	TArray<int32> TeamScores;
	UFUNCTION()
	virtual void OnRep_TeamScore();
	
	UFUNCTION(BlueprintCallable, Category = "Goals")
	virtual int32 GetScoreByTeamID(uint8 TeamID);
	virtual void ScoreGoalFor(int32 TeamID, int32 ScoreAdd);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UParticleSystem* GoalParticleSystem;
	
	





	friend class AKFBBGameModeBase;
};
