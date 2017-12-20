// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KFBB_PlayerPawn.generated.h"


UENUM(BlueprintType)
namespace EKFBB_PlayerState
{
	enum Type
	{
		Ready,
		KnockedDown,
		Stunned,
		StandingUp,
		Moving,
		Exhausted,
	};
}


UCLASS()
class KFBB_API AKFBB_PlayerPawn : public ACharacter
{
	GENERATED_BODY()

	void RegisterWithField();
	void RegisterWithTile(class UKFBB_FieldTile* Tile);


	void SetCooldownTimer(float t) { CooldownTimer = t; }
	void ClearCooldownTimer();
	void OnCooldownTimerExpired();

	void DrawDebugCurrentTile() const;
	void DrawDebugStatus() const;
	FColor GetDebugColor() const;
	
public:
	// Sets default values for this pawn's properties
	AKFBB_PlayerPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadOnly)
	class AKFBB_CoachPC* Coach;
	UPROPERTY(BlueprintReadOnly)
	class AKFBB_Field *Field;
	UPROPERTY(BlueprintReadOnly)
	class UKFBB_FieldTile* CurrentTile;


	UPROPERTY(BlueprintReadOnly)
	float CooldownTimer;

	UFUNCTION(BlueprintCallable)
	bool IsPlayerOnCooldown();

	UFUNCTION(BlueprintCallable)
	bool CanAcceptCommand();

	bool MoveToTileLocation(UKFBB_FieldTile* Tile);
	bool NotifyCommandGiven(UKFBB_FieldTile* DestinationTile);
	void NotifyCommandFailed();
	void NotifyReachedDestination();	

	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	void SetStatus_Moving();
	UFUNCTION(BlueprintCallable)
	bool IsStatusMoving() { return Status == EKFBB_PlayerState::Moving; }
	void SetStatus_Exhausted();
	UFUNCTION(BlueprintCallable)
	bool IsStatusExhausted() { return Status == EKFBB_PlayerState::Exhausted; }
	void SetStatus_KnockedDown();
	UFUNCTION(BlueprintCallable)
	bool IsStatusKnockedDown() { return Status == EKFBB_PlayerState::KnockedDown; }
	void SetStatus_Stunned();
	UFUNCTION(BlueprintCallable)
	bool IsStatusStunned() { return Status == EKFBB_PlayerState::Stunned; }
	void SetStatus_StandUp();
	UFUNCTION(BlueprintCallable)
	bool IsStatusStandingUp() { return Status == EKFBB_PlayerState::StandingUp; }
	void SetStatus_Ready();
	UFUNCTION(BlueprintCallable)
	bool IsStatusReady() { return Status == EKFBB_PlayerState::Ready; }

	UPROPERTY(EditDefaultsOnly)
	float ExhaustedCooldownTime;
	UPROPERTY(EditDefaultsOnly)
	float KnockedDownTime;
	UPROPERTY(EditDefaultsOnly)
	float StunnedTime;
	UPROPERTY(EditDefaultsOnly)
	float StandUpTime;

	EKFBB_PlayerState::Type Status;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int TestStatus;
};
