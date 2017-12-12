// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_PlayerController.h"
#include "KFBB_PlayerPawn.h"

void AKFBB_PlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	MyPlayerPawn = Cast<AKFBB_PlayerPawn>(InPawn);
}

void AKFBB_PlayerController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if(MyPlayerPawn != nullptr)
	{
		MyPlayerPawn->NotifyReachedDestination();
	}
}