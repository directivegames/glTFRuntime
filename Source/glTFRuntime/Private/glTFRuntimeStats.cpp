// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#include "glTFRuntimeStats.h"

#if ENABLE_LOW_LEVEL_MEM_TRACKER
DEFINE_STAT(STAT_LoadAssetsLLM);
DEFINE_STAT(STAT_LoadStaticMeshLLM);
DEFINE_STAT(STAT_LoadSkeletalMeshLLM);
DEFINE_STAT(STAT_FinalizeSkeletalMeshLLM);
DEFINE_STAT(STAT_LoadMaterialLLM);
DEFINE_STAT(STAT_LoadTextureLLM);
DEFINE_STAT(STAT_LoadPrimitivesLLM);
DEFINE_STAT(STAT_LoadAnimationLLM);
DEFINE_STAT(STAT_LoadMorphTargetLLM);
DEFINE_STAT(STAT_InitMorphTargetLLM);
DEFINE_STAT(STAT_BuildTextureLLM);
DEFINE_STAT(STAT_BuildMaterialLLM);
DEFINE_STAT(STAT_LoadJsonLLM);
DEFINE_STAT(STAT_StoreBinaryBufferLLM);
#endif
