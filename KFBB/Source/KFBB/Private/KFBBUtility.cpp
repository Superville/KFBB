// Fill out your copyright notice in the Description page of Project Settings.


#include "KFBBUtility.h"

#include "KFBB_Field.h"
#include "KFBB_FieldTile.h"

FTileInfo::FTileInfo()
{
	TileIdx = INDEX_NONE;
}

FTileInfo::FTileInfo(int32 idx)
{
	TileIdx = idx;
}

FTileInfo::FTileInfo(UKFBB_FieldTile* T)
{
	TileIdx = T ? T->TileIdx : INDEX_NONE;
}

bool FTileInfo::operator==(const FTileInfo T) const
{
	return TileIdx == T.TileIdx;
}

bool FTileInfo::operator!=(const FTileInfo T) const
{
	return TileIdx != T.TileIdx;
}

bool FTileInfo::operator==(const UKFBB_FieldTile* T) const
{
	return T ? (T->TileIdx == TileIdx) : (TileIdx == INDEX_NONE);
}

bool FTileInfo::operator!=(const UKFBB_FieldTile* T) const
{
	return T ? (T->TileIdx != TileIdx) : (TileIdx != INDEX_NONE);
}

FTileInfo& FTileInfo::operator=(const UKFBB_FieldTile* T)
{
	TileIdx = T ? T->TileIdx : INDEX_NONE;
	return *this;
}

FTileDir FTileDir::ConvertToTileDir(FVector2D v)
{
	v = v.GetSafeNormal();
	if (FMath::Abs(v.X) > 0.7f)
	{
		v.X = (v.X < 0) ? -1 : 1;
	}
	else
	{
		v.X = 0;
	}

	if (FMath::Abs(v.Y) > 0.7f)
	{
		v.Y = (v.Y < 0) ? -1 : 1;
	}
	else
	{
		v.Y = 0;
	}

	return FTileDir(v.X, v.Y);
}


//test FGameplayTag UTagLibrary::StatusPlayerHasBall = FGameplayTag::RequestGameplayTag();