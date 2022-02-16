// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "glTFRuntimeParser.h"

#include "glTFRuntimeSettings.generated.h"


UCLASS(Config=Engine, defaultconfig)
class GLTFRUNTIME_API UglTFRuntimeSettings : public UObject
{
	GENERATED_BODY()

public:
	UglTFRuntimeSettings();

	UPROPERTY(EditAnywhere, Config)
	TMap<EglTFRuntimeMaterialType, TSoftObjectPtr<UMaterialInterface>> MetallicRoughnessMaterialsMap;

	UPROPERTY(EditAnywhere, Config)
	TMap<EglTFRuntimeMaterialType, TSoftObjectPtr<UMaterialInterface>> SpecularGlossinessMaterialsMap;
};
