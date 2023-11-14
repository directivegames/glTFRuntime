// Copyright (C) 2023 - Directive Games Limited - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "glTFRuntimeParser.h"

#include "glTFRuntimeAssetActorComponent.generated.h"

class UglTFRuntimeAnimationCurve;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GLTFRUNTIME_API UglTFRuntimeAssetActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UglTFRuntimeAssetActorComponent();

	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "glTFRuntime")
	void SetCurveAnimationByName(const FString& CurveAnimationName);

private:
	friend class AglTFRuntimeAssetActor;

	UPROPERTY()
	TMap<USceneComponent*, UglTFRuntimeAnimationCurve*> CurveBasedAnimations;

	UPROPERTY()
	TMap<USceneComponent*, float> CurveBasedAnimationsTimeTracker;

	TSet<FString> DiscoveredCurveAnimationsNames;

	TMap<TWeakObjectPtr<USceneComponent>, TMap<FString, TRelocatableObjectPtr<UglTFRuntimeAnimationCurve>>> DiscoveredCurveAnimations;
};
