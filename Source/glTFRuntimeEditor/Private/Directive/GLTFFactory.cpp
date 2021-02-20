// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#include "Directive/GLTFFactory.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "glTFRuntimeParser.h"
#include "AssetRegistryModule.h"


UGLTFFactory::UGLTFFactory()
{
	bCreateNew = false;
	bEditAfterNew = false;
	bEditorImport = true;   // binary / general file source
	bText = false;  // text source

	SupportedClass = UStaticMesh::StaticClass();

	Formats.Add(TEXT("gltf;GL Transmission Format"));
	Formats.Add(TEXT("glb;GL Transmission Format (Binary)"));
}

static void SaveObject(const FString& Path, UObject* Object, const FString& ObjectName)
{
	auto Package = CreatePackage(*FString::Printf(TEXT("%s/%s"), *Path, *ObjectName));
	Object->Rename(*ObjectName, Package);
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Object);
}

UObject* UGLTFFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename,
	const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	UObject* Loaded = nullptr;
	FglTFRuntimeConfig Config;
	const auto ParentName = InParent->GetName();
	if (auto Asset = UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(Filename, false, Config))
	{
		const auto Nodes = Asset->GetNodes();
		for (const auto& Node : Nodes)
		{
			if (Node.MeshIndex >= 0)
			{
				if (Node.SkinIndex >= 0)
				{
					FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;
					if (auto SkeletalMesh = Asset->LoadSkeletalMesh(Node.MeshIndex, Node.SkinIndex, SkeletalMeshConfig))
					{
						Loaded = SkeletalMesh;

						if (auto Skeleton = SkeletalMesh->Skeleton)
						{
							SaveObject(ParentName, Skeleton, Node.Name + TEXT("_Skeleton"));							
						}

						SaveObject(ParentName, SkeletalMesh, Node.Name);
					}
				}
				else
				{
					FglTFRuntimeStaticMeshConfig StaticMeshConfig;
					if (auto StaticMesh = Asset->LoadStaticMesh(Node.MeshIndex, StaticMeshConfig))
					{
						Loaded = StaticMesh;
						
						SaveObject(ParentName, StaticMesh, Node.Name);
					}
				}
			}
		}
	}
	return Loaded;
}

void UGLTFFactory::CleanUp()
{
	Super::CleanUp();
}
