// Fill out your copyright notice in the Description page of Project Settings.

#include "KFBB_FieldRenderComponent.h"
#include "KFBB_Field.h"
#include "DrawDebugHelpers.h"


// Sets default values for this component's properties
UKFBB_FieldRenderComponent::UKFBB_FieldRenderComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UKFBB_FieldRenderComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UKFBB_FieldRenderComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

//	DrawDebugField();
}

void UKFBB_FieldRenderComponent::DrawDebugField()
{

}

