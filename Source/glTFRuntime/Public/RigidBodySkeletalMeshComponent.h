// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"

#include "RigidBodySkeletalMeshComponent.generated.h"


class UBodySetup;

UCLASS()
class GLTFRUNTIME_API URigidBodySkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
	
public:
	URigidBodySkeletalMeshComponent();
	bool ShouldCreatePhysicsState() const override;
	void OnCreatePhysicsState() override;
	UBodySetup* GetBodySetup() override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;
};
