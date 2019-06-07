// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "KFBB_PlayerPawn.generated.h"

class UPrimitiveComponent;
class UStaticMeshComponent;
class UInputComponent;
class UDataTable;
struct FTileDir;
class AKFBB_Field;
class UKFBB_FieldTile;
class AKFBB_CoachPC;
class AKFBB_Ball;
class UKFBBAttributeSet;

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
	void RegisterWithTile(UKFBB_FieldTile* Tile);

	void SetCooldownTimer(float t);
	void ClearCooldownTimer();
	void OnCooldownTimerExpired();
	FColor GetCooldownColor() const;

	void DrawDebugCurrentTile() const;
	void DrawDebugStatus() const;
	void DrawDebugPath() const;
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
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Pill = nullptr;

	UPROPERTY(BlueprintReadOnly)
	AKFBB_CoachPC* Coach;
	UPROPERTY(BlueprintReadOnly)
	AKFBB_Field *Field;
	UPROPERTY(BlueprintReadOnly)
	UKFBB_FieldTile* CurrentTile;
	UPROPERTY(BlueprintReadOnly)
	UKFBB_FieldTile* PreviousTile;


	UPROPERTY(BlueprintReadOnly)
	float CooldownTimer;
	UPROPERTY(BlueprintReadOnly)
	float CooldownDuration;

	UFUNCTION(BlueprintCallable)
	virtual bool IsPlayerOnCooldown();

	UFUNCTION(BlueprintCallable, Category = "Grid Pathing")
	virtual bool CanAcceptCommand();
	virtual void NotifyCommandFailed();

	UFUNCTION(BlueprintCallable, Category = "Grid Pathing")
	virtual void NotifyReachedGrid();
	UFUNCTION(BlueprintCallable, Category = "Grid Pathing")
	virtual void NotifyReachedDestinationGrid();

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void KnockDown(FTileDir dir);

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


	AKFBB_Ball* Ball;
	UFUNCTION(BlueprintCallable)
	bool HasBall() const;
	bool CanPickupBall(AKFBB_Ball* ball) const;
	bool TryPickupBall() const;
	void ClaimBall();
	UFUNCTION(BlueprintCallable)
	void AttachBall();
	void FumbleBall();

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EKFBB_PlayerState::Type> Status;
	//test
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int TestStatus;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* PlayerDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerData")
	FString Race;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerData")
	FString Position;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerData")
	int32 Cost;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerData")
	UKFBBAttributeSet* AttribSet;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerSettings")
	bool bRunThroughPlayers;

	void LoadAttributesFromPlayerData(const FKFBB_PlayerData& data);
	UFUNCTION(BlueprintCallable)
	void LoadAttributesByDataName(FString RowName);
};
