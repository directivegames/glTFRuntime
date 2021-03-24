// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#include "RuntimeCollisionFunctionLibrary.h"
#include "ConvexDecompTool.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"


bool URuntimeCollisionFunctionLibrary::GenerateConvexCollisionForStaticMesh(UStaticMesh* StaticMesh, int32 HullCount, int32 MaxHullVerts, int32 HullPrecision)
{
#if WITH_VHACD
	// Check we have a valid StaticMesh
	if (!StaticMesh || !StaticMesh->IsMeshDescriptionValid(0))
	{
		return false;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(GenerateConvexCollision)

	// If RenderData has not been computed yet, do it
	if (!StaticMesh->RenderData)
	{
		StaticMesh->CacheDerivedData();
	}

	const FStaticMeshLODResources& LODModel = StaticMesh->RenderData->LODResources[0];

	// Make vertex buffer
	int32 NumVerts = LODModel.VertexBuffers.StaticMeshVertexBuffer.GetNumVertices();
	TArray<FVector> Verts;
	Verts.Reserve(NumVerts);
	for (int32 i = 0; i < NumVerts; i++)
	{
		Verts.Add(LODModel.VertexBuffers.PositionVertexBuffer.VertexPosition(i));
	}

	// Grab all indices
	TArray<uint32> AllIndices;
	LODModel.IndexBuffer.GetCopy(AllIndices);

	// Only copy indices that have collision enabled
	TArray<uint32> CollidingIndices;
	for (const FStaticMeshSection& Section : LODModel.Sections)
	{
		if (Section.bEnableCollision)
		{
			for (uint32 IndexIdx = Section.FirstIndex; IndexIdx < Section.FirstIndex + (Section.NumTriangles * 3); IndexIdx++)
			{
				CollidingIndices.Add(AllIndices[IndexIdx]);
			}
		}
	}

	// Do not perform any action if we have invalid input
	if (Verts.Num() < 3 || CollidingIndices.Num() < 3)
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

	// Run actual util to do the work (if we have some valid input)
	DecomposeMeshToHulls(BodySetup, Verts, CollidingIndices, HullCount, MaxHullVerts, HullPrecision);

	StaticMesh->bCustomizedCollision = true;	//mark the static mesh for collision customization

	return true;
#else
	return false;
#endif
}
