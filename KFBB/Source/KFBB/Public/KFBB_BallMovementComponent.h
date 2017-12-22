// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "KFBB_BallMovementComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent), ShowCategories = (Velocity))
class KFBB_API UKFBB_BallMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
	
	
public:
	virtual void StopSimulating(const FHitResult& HitResult);
	
	virtual void Test();
};
