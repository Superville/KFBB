// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class KFBBTarget : TargetRules
{
	public KFBBTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange(new string[] { "KFBB" });

		if (bBuildEditor)
		{
			ExtraModuleNames.AddRange(
				new string[]
				{
					"KFBBEditor"
				});
		}
	}
}
