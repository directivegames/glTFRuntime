// Copyright 2020, Roberto De Ioris.

using UnrealBuildTool;

public class RuntimeCollision : ModuleRules
{
    public RuntimeCollision(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
        });

        AddEngineThirdPartyPrivateStaticDependencies(Target, "VHACD");

        if (Target.Platform == UnrealTargetPlatform.Win64 || 
            Target.Platform == UnrealTargetPlatform.Mac ||
            Target.Platform == UnrealTargetPlatform.HoloLens ||
            Target.IsInPlatformGroup(UnrealPlatformGroup.Unix))
        {
            PrivateDefinitions.Add("WITH_VHACD=1");
        }
        else
        {
            PrivateDefinitions.Add("WITH_VHACD=0");
        }
    }
}
