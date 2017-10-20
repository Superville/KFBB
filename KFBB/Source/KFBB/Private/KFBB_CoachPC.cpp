// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_CoachPC.h"
#include "KFBB_Field.h"

//TODO - pull field out blueprint variables and make it a native member, init in BeginPlay
//TODO - finish implementation of PlayerTouchScreen... can be native input event?
//TODO - implement concept of active tile, while there is an active tile mouse move causes others to update?
//TODO - implement concept of click and release, and click and drag?

void AKFBB_CoachPC::BeginPlay()
{
	Super::BeginPlay();
}


void AKFBB_CoachPC::PlayerTouchScreen()
{
	FVector WorldLoc, WorldDir;
	if(DeprojectMousePositionToWorld(WorldLoc, WorldDir))
	{
		auto MyWorld = GetWorld();
		
		FHitResult Hit;
		if (MyWorld->LineTraceSingleByChannel(Hit, WorldLoc, WorldLoc + (WorldDir * 10000.f), ECollisionChannel::ECC_Visibility))
		{
			
		}
	}
}

