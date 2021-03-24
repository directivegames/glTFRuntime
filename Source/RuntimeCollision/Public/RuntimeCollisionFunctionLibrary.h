// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "RuntimeCollisionFunctionLibrary.generated.h"

class UStaticMesh;

UCLASS()
class RUNTIMECOLLISION_API URuntimeCollisionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static bool GenerateConvexCollisionForStaticMesh(UStaticMesh* StaticMesh, int32 HullCount = 4, int32 MaxHullVerts = 16, int32 HullPrecision = 100000);
};
