// Copyright (C) 2023 - Directive Games Limited - All Rights Reserved

#include "Directive/glTFRuntimeAssetActorComponent.h"
#include "glTFRuntimeAnimationCurve.h"


UglTFRuntimeAssetActorComponent::UglTFRuntimeAssetActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UglTFRuntimeAssetActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (TPair<USceneComponent*, UglTFRuntimeAnimationCurve*>& Pair : CurveBasedAnimations)
	{
		// the curve could be null
		if (!Pair.Value)
		{
			continue;
		}
		float MinTime;
		float MaxTime;
		Pair.Value->GetTimeRange(MinTime, MaxTime);

		float CurrentTime = CurveBasedAnimationsTimeTracker[Pair.Key];
		if (CurrentTime > Pair.Value->glTFCurveAnimationDuration)
		{
			CurveBasedAnimationsTimeTracker[Pair.Key] = 0;
			CurrentTime = 0;
		}

		if (CurrentTime >= MinTime)
		{
			FTransform FrameTransform = Pair.Value->GetTransformValue(CurveBasedAnimationsTimeTracker[Pair.Key]);
			Pair.Key->SetRelativeTransform(FrameTransform);
		}
		CurveBasedAnimationsTimeTracker[Pair.Key] += DeltaTime;
	}
}

void UglTFRuntimeAssetActorComponent::SetCurveAnimationByName(const FString& CurveAnimationName)
{
	if (!DiscoveredCurveAnimationsNames.Contains(CurveAnimationName))
	{
		return;
	}

	for (auto& Pair : CurveBasedAnimations)
	{
		const auto Component = Pair.Key;
		if (!Component || !DiscoveredCurveAnimations.Contains(Component))
		{
			continue;
		}
		auto WantedCurveAnimationsMap = DiscoveredCurveAnimations[Component];
		if (WantedCurveAnimationsMap.Contains(CurveAnimationName))
		{
			Pair.Value = WantedCurveAnimationsMap[CurveAnimationName];
			if (!Pair.Value)
			{
				UE_LOG(LogGLTFRuntime, Error, TEXT("Curve animation '%s' for '%s' is no longer in memory"), *CurveAnimationName, *Component->GetPathName());
			}
			CurveBasedAnimationsTimeTracker[Component] = 0;
		}
		else
		{
			Pair.Value = nullptr;
		}

	}
}
