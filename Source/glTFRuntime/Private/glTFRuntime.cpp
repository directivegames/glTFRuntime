// Copyright 2020, Roberto De Ioris.

#include "glTFRuntime.h"

#if 1 // WITH_DIRECTIVE
#include "glTFRuntimeStats.h"
#endif

#define LOCTEXT_NAMESPACE "FglTFRuntimeModule"

void FglTFRuntimeModule::StartupModule()
{
#if 1 // WITH_DIRECTIVE
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)EglTFRuntimeLLMTag::LoadObject, TEXT("glTF Load Object"), GET_STATFNAME(STAT_LoadObjectLLM), NAME_None));
#endif
}

void FglTFRuntimeModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FglTFRuntimeModule, glTFRuntime)