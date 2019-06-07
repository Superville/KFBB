// Fill out your copyright notice in the Description page of Project Settings.


#include "KFBBAttributeSet.h"

#include "Kismet/KismetMathLibrary.h"

UKFBBAttributeSet::UKFBBAttributeSet()
{
	InitAttribute(Stat_Movement, 6);
	InitAttribute(Stat_Strength, 3);
	InitAttribute(Stat_Agility, 3);
	InitAttribute(Stat_Armor, 8);

	InitAttribute(Stat_ExhaustCD, 2.f);
	InitAttribute(Stat_KnockDownCD, 4.f);
	InitAttribute(Stat_StunCD, 6.f);
	InitAttribute(Stat_StandCD, 0.5f);
}

void UKFBBAttributeSet::InitAttribute(FGameplayAttributeData& stat, float value)
{
	stat.SetBaseValue(value);
	stat.SetCurrentValue(value);
}
