// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_AIController.h"
#include "KFBB_PlayerPawn.h"

void AKFBB_AIController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	MyPlayerPawn = Cast<AKFBB_PlayerPawn>(InPawn);
}

void AKFBB_AIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (MyPlayerPawn != nullptr)
	{
		MyPlayerPawn->NotifyReachedDestination();
	}
}