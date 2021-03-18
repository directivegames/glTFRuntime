// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#include "glTFRuntimeSettings.h"

static UMaterialInterface* GetIfValid(const TArray<UMaterialInterface*>& Materials, int index)
{
	if (Materials.IsValidIndex(index))
	{
		return Materials[index];
	}

	return nullptr;
}

UglTFRuntimeSettings::UglTFRuntimeSettings()
{
	auto MaterialLoader = GetDefault<UglTFMaterialLoader>();
	const auto& Materials = MaterialLoader->LoadedMaterials;
	MetallicRoughnessMaterialsMap = {
		{ EglTFRuntimeMaterialType::Opaque, GetIfValid(Materials, 0) },
		{ EglTFRuntimeMaterialType::Translucent, GetIfValid(Materials, 1) },
		{ EglTFRuntimeMaterialType::TwoSided, GetIfValid(Materials, 2) },
		{ EglTFRuntimeMaterialType::TwoSidedTranslucent, GetIfValid(Materials, 3) },
	};

	SpecularGlossinessMaterialsMap = {
		{ EglTFRuntimeMaterialType::Opaque, GetIfValid(Materials, 4) },
		{ EglTFRuntimeMaterialType::Translucent, GetIfValid(Materials, 5) },
		{ EglTFRuntimeMaterialType::TwoSided, GetIfValid(Materials, 6) },
		{ EglTFRuntimeMaterialType::TwoSidedTranslucent, GetIfValid(Materials, 7) },
	};
}
