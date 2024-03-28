// Copyright 2020, Roberto De Ioris.

using UnrealBuildTool;

public class glTFRuntime : ModuleRules
{
    public glTFRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        // OptimizeCode = CodeOptimization.Never;

        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

        PublicIncludePaths.AddRange(
            new string[] {
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "ProceduralMeshComponent"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Json",
                "RenderCore",
                "RHI",
                "ApplicationCore",
                "HTTP",
                "PhysicsCore",
                "Projects",
                "MeshDescription",
                "StaticMeshDescription",
                "AudioExtensions"
            }
            );

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.Add("SkeletalMeshUtilitiesCommon");
            PrivateDependencyModuleNames.Add("UnrealEd");
            PrivateDependencyModuleNames.Add("AssetRegistry");
        }


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
            );

#if true // WITH_DIRECTIVE
        PrivateDependencyModuleNames.Add("RuntimeCollision");
        PublicDefinitions.Add("WITH_ASYNC_API=1");
#else
        PublicDefinitions.Add("WITH_ASYNC_API=1");
#endif
    }
}
