// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KFBB_GameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class KFBB_API UKFBB_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UKFBB_GameplayAbility();

	UPROPERTY(EditDefaultsOnly, Category = "KFBB")
	bool bAutoActivate;
};
