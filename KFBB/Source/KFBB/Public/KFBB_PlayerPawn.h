// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "KFBB_PlayerPawn.generated.h"


UENUM(BlueprintType)
namespace EKFBB_PlayerState
{
	enum Type
	{
		Ready		UMETA(DisplayName = "Ready"),
		KnockedDown UMETA(DisplayName = "KnockedDown"),
		Stunned		UMETA(DisplayName = "Stunned"),
		StandingUp	UMETA(DisplayName = "StandingUp"),
		Moving		UMETA(DisplayName = "Moving"),
		Exhausted	UMETA(DisplayName = "Exhausted"),
		GrabBall	UMETA(DisplayName = "GrabBall"),
	};
}

USTRUCT(BlueprintType)
struct FKFBB_PlayerData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	FKFBB_PlayerData()
		: Race("Undefined")
		, Position("Undefined")
		, Cost(0)
		, MA(0)
		, ST(0)
		, AG(0)
		, AV(0)
	{}

	/** Player Race Title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	FString Race;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	/** Player Position Title */
	FString Position;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 Cost;
	/** Player Attribute: Movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 MA;
	/** Player Attribute: Strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 ST;
	/** Player Attribute: Agility*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 AG;
	/** Player Attribute: Armor Value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 AV;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	float ExhaustCD;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	float KnockCD;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	float StunCD;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	float StandCD;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	TSoftClassPtr<class AKFBB_PlayerPawn> PlayerClassReference;
};


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
	FString GetStatusString() const;
	
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

	void SetStatus(EKFBB_PlayerState::Type newStatus);

	UFUNCTION(BlueprintCallable)
	bool IsStatusMoving() { return Status == EKFBB_PlayerState::Moving; }
	UFUNCTION(BlueprintCallable)
	bool IsStatusExhausted() { return Status == EKFBB_PlayerState::Exhausted; }
	UFUNCTION(BlueprintCallable)
	bool IsStatusKnockedDown() { return Status == EKFBB_PlayerState::KnockedDown; }
	UFUNCTION(BlueprintCallable)
	bool IsStatusStunned() { return Status == EKFBB_PlayerState::Stunned; }
	UFUNCTION(BlueprintCallable)
	bool IsStatusStandingUp() { return Status == EKFBB_PlayerState::StandingUp; }
	UFUNCTION(BlueprintCallable)
	bool IsStatusReady() { return Status == EKFBB_PlayerState::Ready; }
	UFUNCTION(BlueprintCallable)
	bool IsGrabbingBall() { return Status == EKFBB_PlayerState::GrabBall; }


	class AKFBB_Ball* Ball;
	UFUNCTION(BlueprintCallable)
	bool HasBall() const;
	bool CanPickupBall(AKFBB_Ball* ball) const;
	bool TryPickupBall() const;
	void ClaimBall();
	UFUNCTION(BlueprintCallable)
	void AttachBall();
	void FumbleBall();




	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerData)
	float ExhaustedCooldownTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerData)
	float KnockedDownTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerData)
	float StunnedTime;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerData)
	float StandUpTime;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EKFBB_PlayerState::Type> Status;
	//test
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int TestStatus;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UDataTable* PlayerDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	FString Race;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	FString Position;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 Cost;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 MA;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 ST;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 AG;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
	int32 AV;

	void LoadAttributesFromPlayerData(const FKFBB_PlayerData& data);
	UFUNCTION(BlueprintCallable)
	void LoadAttributesByDataName(FString RowName);
};
