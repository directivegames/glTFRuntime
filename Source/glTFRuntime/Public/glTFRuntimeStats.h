// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "HAL/LowLevelMemStats.h"


DECLARE_STATS_GROUP(TEXT("glTFRuntime"), STATGROUP_glTFRuntime, STATCAT_Advanced);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Load Material"), STAT_LoadMaterial, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Load Texture"), STAT_LoadTexture, STATGROUP_glTFRuntime, GLTFRUNTIME_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("From Data"), STAT_FromData, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("From Filename"), STAT_FromFilename, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("From String"), STAT_FromString, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("From Binary"), STAT_FromBinary, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Load Scene"), STAT_LoadScene, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Load Primitives"), STAT_LoadPrimitives, STATGROUP_glTFRuntime, GLTFRUNTIME_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Load Skeletal Mesh"), STAT_LoadSkeletalMesh, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Create Skeletal Mesh From LODs"), STAT_CreateSkeletalMeshFromLODs, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Finalize Skeletal Mesh With LODs"), STAT_FinalizeSkeletalMeshWithLODs, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Build Skeletal Mesh"), STAT_BuildSkeletalMesh, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Save LOD Imported Data"), STAT_SaveLODImportedData, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("USkeletalMesh::Build"), STAT_SkeletalMeshBuild, STATGROUP_glTFRuntime, GLTFRUNTIME_API);

DECLARE_CYCLE_STAT_EXTERN(TEXT("Load Static Mesh"), STAT_LoadStaticMesh, STATGROUP_glTFRuntime, GLTFRUNTIME_API);


enum class EglTFRuntimeLLMTag : LLM_TAG_TYPE
{
	LoadAssets = (uint32)ELLMTag::ProjectTagStart + 10,
	LoadStaticMesh,
	LoadSkeletalMesh,
	FinalizeSkeletalMesh,
	LoadMaterial,
	LoadTexture,
	LoadPrimitives,
};

DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Assets"), STAT_LoadAssetsLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Static Mesh"), STAT_LoadStaticMeshLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Skeletal Mesh"), STAT_LoadSkeletalMeshLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Finalize Skeletal Mesh"), STAT_FinalizeSkeletalMeshLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Material"), STAT_LoadMaterialLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Texture"), STAT_LoadTextureLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Primitives"), STAT_LoadPrimitivesLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
