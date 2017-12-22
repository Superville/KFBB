// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_BallMovementComponent.h"

void UKFBB_BallMovementComponent::StopSimulating(const FHitResult& HitResult)
{
	auto uc = UpdatedComponent;
	auto act = bIsActive;

	Super::StopSimulating(HitResult);
}

void UKFBB_BallMovementComponent::Test()
{
	auto uc = UpdatedComponent;
	auto act = bIsActive;

	int i = 0;
}