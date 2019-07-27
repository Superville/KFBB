// Fill out your copyright notice in the Description page of Project Settings.


#include "KFBB_GameplayAbility.h"

// Engine Includes
#include "GameplayAbilityTypes.h"

UKFBB_GameplayAbility::UKFBB_GameplayAbility()
{
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateYes;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}
