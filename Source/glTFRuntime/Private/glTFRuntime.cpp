// Copyright 2020-2023, Roberto De Ioris.

#include "glTFRuntime.h"

#if 1 // WITH_DIRECTIVE
#include "glTFRuntimeStats.h"
#endif

#define LOCTEXT_NAMESPACE "FglTFRuntimeModule"

#if ENABLE_LOW_LEVEL_MEM_TRACKER // WITH_DIRECTIVE
static void RegisterTag(EglTFRuntimeLLMTag Tag, const FString& Name, const FName& StatName)
{
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)Tag, *Name, StatName, NAME_None));
}
#endif

void FglTFRuntimeModule::StartupModule()
{
#if ENABLE_LOW_LEVEL_MEM_TRACKER // WITH_DIRECTIVE
	RegisterTag(EglTFRuntimeLLMTag::LoadAssets, TEXT("glTF Load Assets"), GET_STATFNAME(STAT_LoadAssetsLLM));
	RegisterTag(EglTFRuntimeLLMTag::LoadStaticMesh, TEXT("glTF Load Static Mesh"), GET_STATFNAME(STAT_LoadStaticMeshLLM));
	RegisterTag(EglTFRuntimeLLMTag::LoadSkeletalMesh, TEXT("glTF Load Skeletal Mesh"), GET_STATFNAME(STAT_LoadSkeletalMeshLLM));
	RegisterTag(EglTFRuntimeLLMTag::FinalizeSkeletalMesh, TEXT("glTF Finalize Skeletal Mesh"), GET_STATFNAME(STAT_FinalizeSkeletalMeshLLM));
	RegisterTag(EglTFRuntimeLLMTag::LoadMaterial, TEXT("glTF Load Material"), GET_STATFNAME(STAT_LoadMaterialLLM));
	RegisterTag(EglTFRuntimeLLMTag::LoadTexture, TEXT("glTF Load Texture"), GET_STATFNAME(STAT_LoadTextureLLM));
	RegisterTag(EglTFRuntimeLLMTag::LoadPrimitives, TEXT("glTF Load Primitives"), GET_STATFNAME(STAT_LoadPrimitivesLLM));
	RegisterTag(EglTFRuntimeLLMTag::LoadAnimation, TEXT("glTF Load Animation"), GET_STATFNAME(STAT_LoadAnimationLLM));
	RegisterTag(EglTFRuntimeLLMTag::LoadMorphTarget, TEXT("glTF Load Morph Target"), GET_STATFNAME(STAT_LoadMorphTargetLLM));
	RegisterTag(EglTFRuntimeLLMTag::InitMorphTarget, TEXT("glTF Init Morph Target"), GET_STATFNAME(STAT_InitMorphTargetLLM));
	RegisterTag(EglTFRuntimeLLMTag::BuildTexture, TEXT("glTF Build Texture"), GET_STATFNAME(STAT_BuildTextureLLM));
	RegisterTag(EglTFRuntimeLLMTag::BuildMaterial, TEXT("glTF Build Material"), GET_STATFNAME(STAT_BuildMaterialLLM));
	RegisterTag(EglTFRuntimeLLMTag::LoadJson, TEXT("glTF Load Json"), GET_STATFNAME(STAT_LoadJsonLLM));
	RegisterTag(EglTFRuntimeLLMTag::StoreBinaryBuffer, TEXT("glTF Store Binary Buffer"), GET_STATFNAME(STAT_StoreBinaryBufferLLM));
#endif
}

void FglTFRuntimeModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FglTFRuntimeModule, glTFRuntime)
