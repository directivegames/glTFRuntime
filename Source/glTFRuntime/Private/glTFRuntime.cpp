// Copyright 2020, Roberto De Ioris.

#include "glTFRuntime.h"

#if 1 // WITH_DIRECTIVE
#include "glTFRuntimeStats.h"
#endif

#define LOCTEXT_NAMESPACE "FglTFRuntimeModule"

void FglTFRuntimeModule::StartupModule()
{
#if 1 // WITH_DIRECTIVE
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)EglTFRuntimeLLMTag::LoadAssets, TEXT("glTF Load Assets"), GET_STATFNAME(STAT_LoadAssetsLLM), NAME_None));
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)EglTFRuntimeLLMTag::LoadStaticMesh, TEXT("glTF Load Static Mesh"), GET_STATFNAME(STAT_LoadStaticMeshLLM), NAME_None));
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)EglTFRuntimeLLMTag::LoadSkeletalMesh, TEXT("glTF Load Skeletal Mesh"), GET_STATFNAME(STAT_LoadSkeletalMeshLLM), NAME_None));
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)EglTFRuntimeLLMTag::FinalizeSkeletalMesh, TEXT("glTF Finalize Skeletal Mesh"), GET_STATFNAME(STAT_FinalizeSkeletalMeshLLM), NAME_None));
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)EglTFRuntimeLLMTag::LoadMaterial, TEXT("glTF Load Material"), GET_STATFNAME(STAT_LoadMaterialLLM), NAME_None));
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)EglTFRuntimeLLMTag::LoadTexture, TEXT("glTF Load Texture"), GET_STATFNAME(STAT_LoadTextureLLM), NAME_None));
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)EglTFRuntimeLLMTag::LoadPrimitives, TEXT("glTF Load Primitives"), GET_STATFNAME(STAT_LoadPrimitivesLLM), NAME_None));
#endif
}

void FglTFRuntimeModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FglTFRuntimeModule, glTFRuntime)