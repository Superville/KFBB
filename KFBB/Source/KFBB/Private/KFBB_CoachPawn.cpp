// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_CoachPawn.h"

// Engine Includes


// KFBB Includes
#include "Coach/KFBBCoachMovementComponent.h"


// Sets default values
AKFBB_CoachPawn::AKFBB_CoachPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UKFBBCoachMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AKFBB_CoachPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKFBB_CoachPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AKFBB_CoachPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
