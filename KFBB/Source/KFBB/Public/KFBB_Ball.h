// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTags.h"
#include "KFBBUtility.h"
#include "AbilitySystemInterface.h"
#include "KFBB_Ball.generated.h"

class UGameplayEffect;
class AKFBB_Field;

UCLASS()
class KFBB_API AKFBB_Ball : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

	void RegisterWithField();
	void RegisterWithTile(class UKFBB_FieldTile* Tile);

	bool bOnGround;


public:	
	// Sets default values for this actor's properties
	AKFBB_Ball();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Reset() override;

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BallSMC = nullptr;


	UPROPERTY()
	AKFBB_Field* Field;
	UPROPERTY()
	FTileInfo CurrentTile;

	UFUNCTION(BlueprintPure, Category = "KFBB")
	FORCEINLINE AKFBB_Field* GetField() const;
	UFUNCTION(BlueprintPure, Category = "KFBB")
	FORCEINLINE UKFBB_FieldTile* GetCurrentTile() const;


	UPROPERTY(Replicated, BlueprintReadOnly, Category="KFBB")
	class AKFBB_PlayerPawn* OwningPlayer;

	void RegisterWithPlayer(class AKFBB_PlayerPawn* P);
	void UnRegisterWithPlayer();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool IsPossessed() const;

	bool CanBePickedUp() const;
	void UpdateCanBePickedUp();
	bool IsFumbled() const;
	bool IsFree() const;
	UFUNCTION(BlueprintCallable, Category = "KFBB | Ball")
	void StopMovement();
	void AdjustBallToTileCenter(float DeltaTime);
	void FumbleBall(UKFBB_FieldTile* DestTile);
	float TimeSinceLastFumble() const;
	
#pragma region GameplayAbilities
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KFBB | Abilities")
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "KFBB | Abilities")
	TArray<TSubclassOf<UKFBB_GameplayAbility>> AbilityList;
	UFUNCTION(BlueprintCallable, Category = "KFBB")
	void GrantAbility(TSubclassOf<UKFBB_GameplayAbility> Ability);
	virtual void InitAbilities();
#pragma endregion

#pragma region GameplayEffects
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "KFBB | Gameplay Effects")
	TSubclassOf<UGameplayEffect> GE_BallState_Possessed = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "KFBB | Gameplay Effects")
	TSubclassOf<UGameplayEffect> GE_BallState_Free = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "KFBB | Gameplay Effects")
	TSubclassOf<UGameplayEffect> GE_BallState_Fumble = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "KFBB | Gameplay Effects")
	TSubclassOf<UGameplayEffect> GE_BallState_Pass = nullptr;
#pragma endregion


	virtual void DrawDebug() const;
	virtual void DrawDebugCurrentTile() const;
};
