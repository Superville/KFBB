// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AbilitySystemInterface.h"
#include "GameplayTags.h"
#include "GameFramework/Character.h"
#include "KFBBUtility.h"
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
class UAbilitySystemComponent;
class UGameplayAbility;

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
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PlayerData)
//	TSoftClassPtr<class AKFBB_PlayerPawn> PlayerClassReference;
};


UCLASS()
class KFBB_API AKFBB_PlayerPawn : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	void RegisterWithField();
	void RegisterWithTile(UKFBB_FieldTile* Tile);

	void SetCooldownTimer(float t);
	void ClearCooldownTimer();
	void OnCooldownTimerExpired();
	FColor GetCooldownColor() const;

	UPROPERTY(Replicated)
	FTileInfo DestinationTile;
	UFUNCTION(BlueprintPure, Category = "KFBB | AI", meta = (AllowPrivateAccess = "true"))
	virtual UKFBB_FieldTile* GetDestinationTile() const;
	UFUNCTION(BlueprintCallable, Category = "KFBB | AI", meta = (AllowPrivateAccess = "true"))
	virtual void SetDestinationTile(FTileInfo Tile);
	UFUNCTION(BlueprintCallable, Category = "KFBB | AI", meta = (AllowPrivateAccess = "true"))
	virtual void ClearDestinationTile();

	UPROPERTY(Replicated)
	TArray<FTileInfo> PathToDestTile;
	UFUNCTION(BlueprintPure, Category = "KFBB | AI", meta = (AllowPrivateAccess = "true"))
	virtual bool HasPathToDestTile();
	UFUNCTION(BlueprintCallable, Category = "KFBB | AI", meta = (AllowPrivateAccess = "true"))
	virtual void SetPathToDestTile(TArray<FTileInfo>& PathToDest);
	UFUNCTION(BlueprintCallable, Category = "KFBB | AI", meta = (AllowPrivateAccess = "true"))
	virtual void ClearPathToDestTile();

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Pill = nullptr;

	UFUNCTION(BlueprintImplementableEvent, Category = "KFBB", meta=(DisplayName="UpdateTeamColor"))
	void BP_UpdateTeamColor();
	bool CanBeSelected(AKFBB_CoachPC* inCoach) const;
	bool IsMyCoach(AKFBB_CoachPC* inCoach) const;
	bool IsSameTeam(AKFBB_PlayerPawn* Other) const;

	UPROPERTY()
	FTileInfo CurrentTile;
	UPROPERTY()
	FTileInfo PreviousTile;
	UPROPERTY()
	FTileInfo NextTile;
	UPROPERTY()
	AKFBB_Field *Field;

	UFUNCTION(BlueprintPure, Category = "KFBB")
	FORCEINLINE AKFBB_Field* GetField() const;
	UFUNCTION(BlueprintPure, Category = "KFBB")
	FORCEINLINE UKFBB_FieldTile* GetCurrentTile() const;
	UFUNCTION(BlueprintPure, Category = "KFBB")
	FORCEINLINE UKFBB_FieldTile* GetPreviousTile() const;
	UFUNCTION(BlueprintPure, Category = "KFBB")
	FORCEINLINE UKFBB_FieldTile* GetNextTile() const;



	//todo replace with GE cooldown
	UPROPERTY(BlueprintReadOnly)
	float CooldownTimer;
	UPROPERTY(BlueprintReadOnly)
	float CooldownDuration;

	UFUNCTION(BlueprintCallable)
	virtual bool IsPlayerOnCooldown();

	UFUNCTION(BlueprintCallable, Category = "KFBB | Grid Pathing")
	virtual bool CanAcceptCommand();
	virtual void NotifyCommandFailed();

	UFUNCTION(BlueprintCallable, Category = "KFBB | Grid Pathing")
	virtual void NotifyReachedGrid();
	UFUNCTION(BlueprintCallable, Category = "KFBB | Grid Pathing")
	virtual void NotifyReachedDestinationGrid();
	UFUNCTION(BlueprintPure, Category = "KFBB | Grid Pathing")
	UKFBB_FieldTile* GetFirstGrid() const;
	UFUNCTION(BlueprintCallable, Category = "KFBB | Grid Pathing")
	void RemoveFirstGrid();

	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void KnockDown(FTileDir dir);

	UFUNCTION(BlueprintCallable, Category="KFBB")
	void SetStatus(EKFBB_PlayerState::Type newStatus);

	UFUNCTION(BlueprintCallable, Category = "KFBB")
	bool IsStatusMoving() { return Status == EKFBB_PlayerState::Moving; }
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	bool IsStatusExhausted() { return Status == EKFBB_PlayerState::Exhausted; }
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	bool IsStatusKnockedDown() { return Status == EKFBB_PlayerState::KnockedDown; }
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	bool IsStatusStunned() { return Status == EKFBB_PlayerState::Stunned; }
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	bool IsStatusStandingUp() { return Status == EKFBB_PlayerState::StandingUp; }
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	bool IsStatusReady() { return Status == EKFBB_PlayerState::Ready; }
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	bool IsGrabbingBall() { return Status == EKFBB_PlayerState::GrabBall; }


	AKFBB_Ball* Ball;
	UFUNCTION(BlueprintCallable)
	bool HasBall() const;
	UFUNCTION(BlueprintCallable)
	bool CanPickupBall(AKFBB_Ball* ball) const;
	UFUNCTION(BlueprintCallable)
	bool TryPickupBall() const;
	UFUNCTION(BlueprintCallable, Category="KFBB")
	void ClaimBall(AKFBB_Ball* BallToClaim);
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	void AttachBall(AKFBB_Ball* BallToAttach);
	UFUNCTION(BlueprintCallable)
	void FumbleBall(AKFBB_Ball* BallToFumble);

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EKFBB_PlayerState::Type> Status;

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	void GrantAbility(TSubclassOf<UGameplayAbility> Ability);

	UFUNCTION(BlueprintCallable, Category = "KFBB | Stats")
	virtual int32 GetStat_Movement();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTag ProhibitCommandsTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tags")
	FGameplayTag TriggerPickupAbilityTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tags")
	FGameplayTag HasBallTag;

	void LoadAttributesFromPlayerData(const FKFBB_PlayerData& data);
	UFUNCTION(BlueprintCallable)
	void LoadAttributesByDataName(FString RowName);

	UPROPERTY(ReplicatedUsing="OnRep_TeamID")
	uint8 TeamID = 255;
	uint8 DisplayTeamID = 255;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "KFBB")
	uint8 GetTeamID() const;
	void SetTeamID(uint8 teamID);
	UFUNCTION()
	void OnRep_TeamID();
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "KFBB")
	uint8 GetDisplayTeamID() const;

	friend class AKFBB_AIController;
};
