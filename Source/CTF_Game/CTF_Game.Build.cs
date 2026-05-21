// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CTF_Game : ModuleRules
{
	public CTF_Game(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"CTF_Game",
			"CTF_Game/Variant_Platforming",
			"CTF_Game/Variant_Platforming/Animation",
			"CTF_Game/Variant_Combat",
			"CTF_Game/Variant_Combat/AI",
			"CTF_Game/Variant_Combat/Animation",
			"CTF_Game/Variant_Combat/Gameplay",
			"CTF_Game/Variant_Combat/Interfaces",
			"CTF_Game/Variant_Combat/UI",
			"CTF_Game/Variant_SideScrolling",
			"CTF_Game/Variant_SideScrolling/AI",
			"CTF_Game/Variant_SideScrolling/Gameplay",
			"CTF_Game/Variant_SideScrolling/Interfaces",
			"CTF_Game/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
