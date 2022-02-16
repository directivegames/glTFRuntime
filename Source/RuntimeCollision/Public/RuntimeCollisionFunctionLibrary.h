// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/GCObject.h"
#include "Tickable.h"

#include "RuntimeCollisionFunctionLibrary.generated.h"

class UStaticMesh;
class IDecomposeMeshToHullsAsync;

UCLASS()
class RUNTIMECOLLISION_API URuntimeCollisionFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static bool GenerateConvexCollisionForStaticMesh(UStaticMesh* StaticMesh, int32 HullCount = 4, int32 MaxHullVerts = 16, int32 HullPrecision = 100000);
};


DECLARE_DELEGATE(FOnConvexCollisionGenerationFinished);

class RUNTIMECOLLISION_API FAsyncConvexCollisionGenerator : public FGCObject, public FTickableGameObject, public TSharedFromThis<FAsyncConvexCollisionGenerator>
{
public:
	virtual ~FAsyncConvexCollisionGenerator();

	static bool GenerateCollisionForStaticMesh(UStaticMesh* InStaticMesh, int32 HullCount, int32 MaxHullVerts, int32 HullPrecision, FOnConvexCollisionGenerationFinished InCallback);

	// FGCObject
	void AddReferencedObjects(FReferenceCollector& Collector) override;
	FString GetReferencerName() const { return TEXT("FAsyncConvexCollisionGenerator");  }
	// !FGCObject

	// FTickableGameObject
	bool IsTickable() const override;
	TStatId GetStatId() const override;
	void Tick(float DeltaTime) override;
	// !FTickableGameObject

private:
	void Cleanup();
	bool DoGenerateCollisionForStaticMesh(UStaticMesh* InStaticMesh, int32 HullCount, int32 MaxHullVerts, int32 HullPrecision, FOnConvexCollisionGenerationFinished InCallback);

private:
	UStaticMesh* StaticMesh = nullptr;
	IDecomposeMeshToHullsAsync* Task = nullptr;
	double BeginTime = 0.f;
	FOnConvexCollisionGenerationFinished Callback;
};
