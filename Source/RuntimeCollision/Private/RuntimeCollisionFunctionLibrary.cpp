// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#include "RuntimeCollisionFunctionLibrary.h"
#include "ConvexDecompTool.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "RuntimeCollision.h"


// Internally we keep a record of all the running generators, so that the caller doesn't need to keep a copy to make them alive
static TArray<TSharedPtr<FAsyncConvexCollisionGenerator>> AllGenerators;


static bool PrepareStaticMeshForCollisionGeneration(UStaticMesh* StaticMesh, TArray<FVector>& Vertices, TArray<uint32>& Indices)
{
	check(StaticMesh);

#if WITH_EDITORONLY_DATA
	if (!StaticMesh->IsMeshDescriptionValid(0))
	{
		return false;
	}
#endif

#if WITH_EDITOR
		// If RenderData has not been computed yet, do it
		if (!StaticMesh->RenderData)
		{
			StaticMesh->CacheDerivedData();
		}
#endif

	const FStaticMeshLODResources& LODModel = StaticMesh->RenderData->LODResources[0];

	// Make vertex buffer
	int32 NumVerts = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices();
	Vertices.Reserve(NumVerts);
	for (int32 i = 0; i < NumVerts; i++)
	{
		Vertices.Add(LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(i));
	}

	// Grab all indices
	TArray<uint32> AllIndices;
	LODModel.IndexBuffer.GetCopy(AllIndices);

	// Only copy indices that have collision enabled
	for (const FStaticMeshSection& Section : LODModel.Sections)
	{
		if (Section.bEnableCollision)
		{
			for (uint32 IndexIdx = Section.FirstIndex; IndexIdx < Section.FirstIndex + (Section.NumTriangles * 3); IndexIdx++)
			{
				Indices.Add(AllIndices[IndexIdx]);
			}
		}
	}

	// Do not perform any action if we have invalid input
	if (Vertices.Num() < 3 || Indices.Num() < 3)
	{
		return false;
	}

	// Get the BodySetup we are going to put the collision into
	UBodySetup* BodySetup = StaticMesh->BodySetup;
	if (BodySetup)
	{
		BodySetup->RemoveSimpleCollision();
	}
	else
	{
		// Otherwise, create one here.
		StaticMesh->CreateBodySetup();
		BodySetup = StaticMesh->BodySetup;
	}

	return true;
}

bool URuntimeCollisionFunctionLibrary::GenerateConvexCollisionForStaticMesh(UStaticMesh* StaticMesh, int32 HullCount, int32 MaxHullVerts, int32 HullPrecision)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(URuntimeCollisionFunctionLibrary::GenerateConvexCollisionForStaticMesh);

#if WITH_VHACD
	if (!StaticMesh)
	{
		return false;
	}

	TArray<FVector> Verts;
	TArray<uint32> CollidingIndices;

	if (!PrepareStaticMeshForCollisionGeneration(StaticMesh, Verts, CollidingIndices))
	{
		return false;
	}

	// Run actual util to do the work (if we have some valid input)
	DecomposeMeshToHulls(StaticMesh->BodySetup, Verts, CollidingIndices, HullCount, MaxHullVerts, HullPrecision);

#if WITH_EDITORONLY_DATA
	StaticMesh->bCustomizedCollision = true;	//mark the static mesh for collision customization
#endif

	return true;
#else
	return false;
#endif
}

FAsyncConvexCollisionGenerator::~FAsyncConvexCollisionGenerator()
{
	Cleanup();
}

void FAsyncConvexCollisionGenerator::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(StaticMesh);
}

bool FAsyncConvexCollisionGenerator::IsTickable() const
{
	return StaticMesh && Task;
}

TStatId FAsyncConvexCollisionGenerator::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncConvexCollisionGenerator, STATGROUP_Tickables);
}

void FAsyncConvexCollisionGenerator::Tick(float DeltaTime)
{
	if (Task)
	{
		if (Task->IsComplete())
		{
			const auto ElapsedTime = FPlatformTime::Seconds() - BeginTime;
			UE_LOG(LogRuntimeCollision, Log, TEXT("FAsyncConvexCollisionGenerator: collision generated for [%s] in %.2f ms"), 
				*StaticMesh->GetName(), (float)ElapsedTime * 1000.f);
			
			Callback.ExecuteIfBound();

			AllGenerators.RemoveSingleSwap(AsShared());

			Cleanup();
		}
	}
}

void FAsyncConvexCollisionGenerator::Cleanup()
{
	StaticMesh = nullptr;
	if (Task)
	{
		Task->Release();
		Task = nullptr;
	}
	Callback = {};
}

bool FAsyncConvexCollisionGenerator::GenerateCollisionForStaticMesh(UStaticMesh* InStaticMesh, int32 HullCount, int32 MaxHullVerts, int32 HullPrecision, FOnConvexCollisionGenerationFinished InCallback)
{
	auto Generator = MakeShared<FAsyncConvexCollisionGenerator>();
	if (!Generator->DoGenerateCollisionForStaticMesh(InStaticMesh, HullCount, MaxHullVerts, HullPrecision, InCallback))
	{
		return false;
	}
	AllGenerators.Add(Generator);
	return true;
}

bool FAsyncConvexCollisionGenerator::DoGenerateCollisionForStaticMesh(UStaticMesh* InStaticMesh, int32 HullCount, int32 MaxHullVerts, int32 HullPrecision, FOnConvexCollisionGenerationFinished InCallback)
{
#if WITH_VHACD
	if (!InStaticMesh)
	{
		return false;
	}

	if (StaticMesh || Task)
	{
		UE_LOG(LogRuntimeCollision, Error, TEXT("FAsyncConvexCollisionGenerator::GenerateCollisionForStaticMesh: the previous task has not finished yet!"));
		return false;
	}

	StaticMesh = InStaticMesh;
	Task = CreateIDecomposeMeshToHullAsync();
	if (!Task)
	{
		return false;
	}
	
	TArray<FVector> Verts;
	TArray<uint32> CollidingIndices;

	if (!PrepareStaticMeshForCollisionGeneration(StaticMesh, Verts, CollidingIndices))
	{
		Cleanup();
		return false;
	}

	if (!Task->DecomposeMeshToHullsAsyncBegin(StaticMesh->BodySetup, Verts, CollidingIndices, HullCount, MaxHullVerts, HullPrecision))
	{
		Cleanup();
		return false;
	}

	BeginTime = FPlatformTime::Seconds();
	Callback = InCallback;
	return true;

#else // WITH_VHACD
	return false;
#endif // WITH_VHACD
}
