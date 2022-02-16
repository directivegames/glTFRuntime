// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "HAL/LowLevelMemStats.h"


DECLARE_STATS_GROUP(TEXT("glTFRuntime"), STATGROUP_glTFRuntime, STATCAT_Advanced);

#if ENABLE_LOW_LEVEL_MEM_TRACKER
enum class EglTFRuntimeLLMTag : LLM_TAG_TYPE
{
	LoadAssets = (uint32)ELLMTag::ProjectTagStart + 10,
	LoadStaticMesh,
	LoadSkeletalMesh,
	FinalizeSkeletalMesh,
	LoadMaterial,
	LoadTexture,
	LoadPrimitives,
	LoadAnimation,
	LoadMorphTarget,
	InitMorphTarget,
	BuildTexture,
	BuildMaterial,
	LoadJson,
	StoreBinaryBuffer,
};

DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Assets"), STAT_LoadAssetsLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Static Mesh"), STAT_LoadStaticMeshLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Skeletal Mesh"), STAT_LoadSkeletalMeshLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Finalize Skeletal Mesh"), STAT_FinalizeSkeletalMeshLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Material"), STAT_LoadMaterialLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Texture"), STAT_LoadTextureLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Primitives"), STAT_LoadPrimitivesLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Animation"), STAT_LoadAnimationLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Morph Target"), STAT_LoadMorphTargetLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Init Morph Target"), STAT_InitMorphTargetLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Build Texture"), STAT_BuildTextureLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Build Material"), STAT_BuildMaterialLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Load Json"), STAT_LoadJsonLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
DECLARE_LLM_MEMORY_STAT_EXTERN(TEXT("glTF Store Binary Buffer"), STAT_StoreBinaryBufferLLM, STATGROUP_glTFRuntime, GLTFRUNTIME_API);
#endif
