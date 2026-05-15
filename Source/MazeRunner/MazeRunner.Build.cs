// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MazeRunner : ModuleRules
{
	public MazeRunner(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
            "ModularGameplay",
			"ModularGameplayActors",
            "NavigationSystem",
            "GameFeatures",
            "GameplayTags",
			"GameplayTasks",
			"GameplayAbilities",
			"NetCore",
			"HTTP",
			"Json",
			"JsonUtilities"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MazeRunner"
			/*"MazeRunner/OriginalSample/",
			"MazeRunner/OriginalSample/Variant_Horror",
			"MazeRunner/OriginalSample/Variant_Horror/UI",
			"MazeRunner/OriginalSample/Variant_Shooter",
			"MazeRunner/OriginalSample/Variant_Shooter/AI",
			"MazeRunner/OriginalSample/Variant_Shooter/UI",
			"MazeRunner/OriginalSample/Variant_Shooter/Weapons"*/
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
