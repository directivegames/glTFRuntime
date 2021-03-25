// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#include "glTFRuntimeStats.h"

DEFINE_STAT(STAT_LoadMaterial);
DEFINE_STAT(STAT_LoadTexture);
DEFINE_STAT(STAT_FromData);
DEFINE_STAT(STAT_FromFilename);
DEFINE_STAT(STAT_FromString);
DEFINE_STAT(STAT_FromBinary);
DEFINE_STAT(STAT_LoadScene);
DEFINE_STAT(STAT_LoadPrimitives);
DEFINE_STAT(STAT_LoadPrimitive);
DEFINE_STAT(STAT_BuildFromAccessorField);
DEFINE_STAT(STAT_GetAccessor);
DEFINE_STAT(STAT_GetBufferView);
DEFINE_STAT(STAT_GetBuffer);

DEFINE_STAT(STAT_LoadSkeletalMesh);
DEFINE_STAT(STAT_CreateSkeletalMeshFromLODs);
DEFINE_STAT(STAT_FinalizeSkeletalMeshWithLODs);
DEFINE_STAT(STAT_BuildSkeletalMesh);
DEFINE_STAT(STAT_SaveLODImportedData);
DEFINE_STAT(STAT_SkeletalMeshBuild);
DEFINE_STAT(STAT_LoadStaticMesh);
DEFINE_STAT(STAT_BuildTexture);
DEFINE_STAT(STAT_BuildMaterial);
DEFINE_STAT(STAT_BeginPlay);
DEFINE_STAT(STAT_LoadNode);
DEFINE_STAT(STAT_ProcessNode);
DEFINE_STAT(STAT_AddStaticMeshComponent);
DEFINE_STAT(STAT_AddSkeletalMeshComponent);
DEFINE_STAT(STAT_AddCameraComponent);
DEFINE_STAT(STAT_AddSceneMeshComponent);
DEFINE_STAT(STAT_LoadAnimationCurves);
DEFINE_STAT(STAT_LoadSkeletalAnimation);
DEFINE_STAT(STAT_LoadAnimation_Internal);
DEFINE_STAT(STAT_GenerateConvexCollision);

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
