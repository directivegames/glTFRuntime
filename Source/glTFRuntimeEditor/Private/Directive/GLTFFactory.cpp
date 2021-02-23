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

template<class T>
static void SaveObjects(const TMap<int32, T>& Objects, const FString& Path, const FString& NodeName, TArray<UObject*>& SavedObjects, const FString& ObjectType)
{
	for (const auto& Itr : Objects)
	{
		auto Object = Itr.Value;
		if (!SavedObjects.Contains(Object))
		{
			SaveObject(Path, Object, FString::Printf(TEXT("%s_%s_%d"), *NodeName, *ObjectType, Itr.Key));
			SavedObjects.Add(Object);
		}
	}
}

static void SaveAssets(UglTFRuntimeAsset* Asset, const FString& Path, const FString& NodeName, TArray<UObject*>& SavedObjects)
{
	if (!Asset)
	{
		return;
	}

	const auto& Parser = Asset->GetParser();
	if (Parser)
	{
		SaveObjects(Parser->GetLoadedTextures(), Path, NodeName, SavedObjects, TEXT("Texture"));
		SaveObjects(Parser->GetLoadedMaterials(), Path, NodeName, SavedObjects, TEXT("Material"));
		SaveObjects(Parser->GetLoadedStaticMeshes(), Path, NodeName, SavedObjects, TEXT("StaticMesh"));
		SaveObjects(Parser->GetLoadedSkeletons(), Path, NodeName, SavedObjects, TEXT("Skeleton"));
		SaveObjects(Parser->GetLoadedSkeletalMeshes(), Path, NodeName, SavedObjects, TEXT("SkeletalMesh"));
	}
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
		TArray<UObject*> SavedObjects;
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

						SaveAssets(Asset, ParentName, Node.Name, SavedObjects);
					}
				}
				else
				{
					FglTFRuntimeStaticMeshConfig StaticMeshConfig;
					if (auto StaticMesh = Asset->LoadStaticMesh(Node.MeshIndex, StaticMeshConfig))
					{
						Loaded = StaticMesh;

						SaveAssets(Asset, ParentName, Node.Name, SavedObjects);
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
