// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#include "RigidBodySkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/CollisionProfile.h"


URigidBodySkeletalMeshComponent::URigidBodySkeletalMeshComponent()
{
	SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
}

bool URigidBodySkeletalMeshComponent::ShouldCreatePhysicsState() const
{
	return USkinnedMeshComponent::ShouldCreatePhysicsState();
}

void URigidBodySkeletalMeshComponent::OnCreatePhysicsState()
{
	USkinnedMeshComponent::OnCreatePhysicsState();
}

UBodySetup* URigidBodySkeletalMeshComponent::GetBodySetup()
{
	if (SkeletalMesh)
	{
		return SkeletalMesh->BodySetup;
	}

	return nullptr;
}

void URigidBodySkeletalMeshComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	USkinnedMeshComponent::OnUpdateTransform(UpdateTransformFlags, Teleport);
}

void URigidBodySkeletalMeshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST) && 0
	if (SkeletalMesh && SkeletalMesh->BodySetup)
	{
		const auto ComponentTransform = K2_GetComponentToWorld();
		for (const auto& Box : SkeletalMesh->BodySetup->AggGeom.BoxElems)
		{
			FVector Scale(Box.X, Box.Y, Box.Z);
			Scale = Scale * ComponentTransform.GetScale3D() / 2.f;
			UKismetSystemLibrary::DrawDebugBox(this, ComponentTransform.TransformPosition(Box.Center), Scale, FLinearColor::Green, 
				ComponentTransform.TransformRotation(Box.Rotation.Quaternion()).Rotator());
		}		
	}
#endif
}
