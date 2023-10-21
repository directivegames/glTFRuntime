// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#include "glTFRuntimeParser.h"

const TArray<TSharedPtr<FJsonValue>>* FglTFRuntimeParser::CheckJsonIndex(TSharedRef<FJsonObject> JsonObject, const FString& FieldName, const int32 Index)
{
	if (Index < 0)
	{
		return nullptr;
	}

	const TArray<TSharedPtr<FJsonValue>>* JsonArray = nullptr;
	JsonObject->TryGetArrayField(FieldName, JsonArray);
	if (!JsonArray)
	{
		return nullptr;
	}

	if (Index >= JsonArray->Num())
	{
		return nullptr;
	}

	return JsonArray;
}

TSharedPtr<FJsonObject> FglTFRuntimeParser::GetJsonObjectFromIndex(TSharedRef<FJsonObject> JsonObject, const FString& FieldName, const int32 Index)
{
	if (auto JsonArray = CheckJsonIndex(JsonObject, FieldName, Index))
	{
		return (*JsonArray)[Index]->AsObject();
	}

	return nullptr;
}

bool FglTFRuntimeParser::LoadAnimation_Internal(TSharedRef<FJsonObject> JsonAnimationObject, float& Duration, FString& Name, TFunctionRef<void(const FglTFRuntimeNode& Node, const FString& Path, const FglTFRuntimeAnimationCurve& Curve)> Callback, TFunctionRef<bool(const FglTFRuntimeNode& Node)> NodeFilter, const TArray<FglTFRuntimePathItem>& OverrideTrackNameFromExtension)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadAnimation_Internal);

	if (!ParsedAnimationCurvesCache.Contains(JsonAnimationObject))
	{
		Name = GetJsonObjectString(JsonAnimationObject, TEXT("name"), {});
		const TArray<TSharedPtr<FJsonValue>>* JsonSamplers;
		if (!JsonAnimationObject->TryGetArrayField(TEXT("samplers"), JsonSamplers))
		{
			return false;
		}

		Duration = 0.f;

		TArray<FglTFRuntimeAnimationCurve> Samplers;

		for (int32 SamplerIndex = 0; SamplerIndex < JsonSamplers->Num(); SamplerIndex++)
		{
			TSharedPtr<FJsonObject> JsonSamplerObject = (*JsonSamplers)[SamplerIndex]->AsObject();
			if (!JsonSamplerObject)
			{
				return false;
			}

			FglTFRuntimeAnimationCurve AnimationCurve;

			if (!BuildFromAccessorField(JsonSamplerObject.ToSharedRef(), TEXT("input"), AnimationCurve.Timeline, { EGLTFComponentType::Float }, INDEX_NONE, false, nullptr))
			{
				AddError("LoadAnimation_Internal()", FString::Printf(TEXT("Unable to retrieve \"input\" from sampler %d"), SamplerIndex));
				return false;
			}

			if (!BuildFromAccessorField(JsonSamplerObject.ToSharedRef(), TEXT("output"), AnimationCurve.Values, { 1, 3, 4 }, { EGLTFComponentType::Float, EGLTFComponentType::Int8, EGLTFComponentType::UInt8, EGLTFComponentType::Int16, EGLTFComponentType::UInt16 }, INDEX_NONE, true, nullptr))
			{
				AddError("LoadAnimation_Internal()", FString::Printf(TEXT("Unable to retrieve \"output\" from sampler %d"), SamplerIndex));
				return false;
			}

			FString SamplerInterpolation;
			if (!JsonSamplerObject->TryGetStringField(TEXT("interpolation"), SamplerInterpolation))
			{
				SamplerInterpolation = TEXT("LINEAR");
			}

			// get animation valid duration
			for (float Time : AnimationCurve.Timeline)
			{
				if (Time > Duration)
				{
					Duration = Time;
				}
			}

			// extract tangents and value (unfortunately Unreal does not support Cubic Splines for skeletal animations)
			if (SamplerInterpolation == TEXT("CUBICSPLINE"))
			{
				TArray<FVector4> CubicValues;
				for (int32 TimeIndex = 0; TimeIndex < AnimationCurve.Timeline.Num(); TimeIndex++)
				{
					// gather A, V and B
					FVector4 InTangent = AnimationCurve.Values[TimeIndex * 3];
					FVector4 Value = AnimationCurve.Values[TimeIndex * 3 + 1];
					FVector4 OutTangent = AnimationCurve.Values[TimeIndex * 3 + 2];

					AnimationCurve.InTangents.Add(InTangent);
					AnimationCurve.OutTangents.Add(OutTangent);
					CubicValues.Add(Value);
				}

				AnimationCurve.Values = CubicValues;
			}

			Samplers.Add(AnimationCurve);
		}

		auto Result = MakeShared<FParsedAnimationCurves>();		
		Result->Name = Name;
		Result->Duration = Duration;
		Result->Samplers = Samplers;
		// cache the channels
		const TArray<TSharedPtr<FJsonValue>>* JsonChannels = nullptr;
		JsonAnimationObject->TryGetArrayField(TEXT("channels"), JsonChannels);
		if (JsonChannels)
		{
			for (const auto& Value : *JsonChannels)
			{
				if (!Value)
				{
					continue;
				}

				const auto JsonChannelObject = Value->AsObject();
				if (!JsonChannelObject)
				{
					continue;
				}

				FChannelObject ChannelObject;
				if (!JsonChannelObject->TryGetNumberField(TEXT("sampler"), ChannelObject.Sampler))
				{
					continue;
				}

				const TSharedPtr<FJsonObject>* JsonTargetObject;
				if (!JsonChannelObject->TryGetObjectField(TEXT("target"), JsonTargetObject))
				{
					continue;
				}

				int64 NodeIndex;
				if (!(*JsonTargetObject)->TryGetNumberField(TEXT("node"), NodeIndex))
				{
					continue;
				}

				if (!(*JsonTargetObject)->TryGetStringField(TEXT("path"), ChannelObject.TargetPath))
				{
					continue;
				}

				if (!LoadNode(NodeIndex, ChannelObject.TargetNode))
				{
					continue;
				}

				ChannelObject.JsonTargetObject = *JsonTargetObject;
				Result->JsonChannels.Add(ChannelObject);
			}
		}

		ParsedAnimationCurvesCache.Add(JsonAnimationObject, Result);
	}

	const auto& Cache = ParsedAnimationCurvesCache[JsonAnimationObject];
	Name = Cache->Name;
	Duration = Cache->Duration;
	const auto& Samplers = Cache->Samplers;
	if (Cache->JsonChannels.IsEmpty())
	{
		return false;
	}

	for (const auto& ChannelObject : Cache->JsonChannels)
	{
		const auto& Sampler = ChannelObject.Sampler;
		if (Sampler >= Samplers.Num())
		{
			return false;
		}
#if 0
		const auto JsonTargetObject = &ChannelObject.JsonTargetObject;
		FglTFRuntimeNode Node;
		if (OverrideTrackNameFromExtension.Num() > 0)
		{
			const TSharedPtr<FJsonObject>* JsonTargetExtensions;
			if ((*JsonTargetObject)->TryGetObjectField("extensions", JsonTargetExtensions))
			{
				TSharedPtr<FJsonValue> JsonTrackName = GetJSONObjectFromRelativePath(JsonTargetExtensions->ToSharedRef(), OverrideTrackNameFromExtension);
				if (JsonTrackName)
				{
					JsonTrackName->TryGetString(Node.Name);
				}
			}
		}

		if (Node.Name.IsEmpty())
		{
			Node = ChannelObject.TargetNode;
		}
#else
		ensure(OverrideTrackNameFromExtension.IsEmpty());
		const auto& Node = ChannelObject.TargetNode;
#endif
		if (!NodeFilter(Node))
		{
			continue;
		}

		Callback(Node, ChannelObject.TargetPath, Samplers[Sampler]);
	}

	return true;
}

TArray<UglTFRuntimeAnimationCurve*> FglTFRuntimeParser::LoadAllNodeAnimationCurves(const int32 NodeIndex)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadAllNodeAnimationCurves);

	if (auto Record = AnimationCurvesCache.Find(NodeIndex))
	{
		return *Record;
	}

	FglTFRuntimeNode Node;
	if (!LoadNode(NodeIndex, Node))
	{
		return {};
	}

	const TArray<TSharedPtr<FJsonValue>>* JsonAnimations;
	if (!Root->TryGetArrayField(TEXT("animations"), JsonAnimations))
	{
		return {};
	}

	UglTFRuntimeAnimationCurve* AnimationCurve = nullptr;

	FTransform OriginalTransform = FTransform(SceneBasis * Node.Transform.ToMatrixWithScale() * SceneBasis.Inverse());

	bool bAnimationFound = false;

	auto Callback = [&](const FglTFRuntimeNode& Node, const FString& Path, const FglTFRuntimeAnimationCurve& Curve)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadNodeAnimation_Callback);

			if (Path == TEXT("translation"))
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadNodeAnimation_AddTranslation);
				if (Curve.Timeline.Num() != Curve.Values.Num())
				{
					AddError(TEXT("LoadAllNodeAnimationCurves()"), FString::Printf(TEXT("Animation input/output mismatch (%d/%d) for translation on node %d"), Curve.Timeline.Num(), Curve.Values.Num(), Node.Index));
					return;
				}
				for (int32 TimeIndex = 0; TimeIndex < Curve.Timeline.Num(); TimeIndex++)
				{
					AnimationCurve->AddLocationValue(Curve.Timeline[TimeIndex], Curve.Values[TimeIndex] * SceneScale, ERichCurveInterpMode::RCIM_Linear);
				}
			}
			else if (Path == TEXT("rotation"))
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadNodeAnimation_AddRotation);
				if (Curve.Timeline.Num() != Curve.Values.Num())
				{
					AddError(TEXT("LoadAllNodeAnimationCurves()"), FString::Printf(TEXT("Animation input/output mismatch (%d/%d) for rotation on node %d"), Curve.Timeline.Num(), Curve.Values.Num(), Node.Index));
					return;
				}
				for (int32 TimeIndex = 0; TimeIndex < Curve.Timeline.Num(); TimeIndex++)
				{
					FVector4 RotationValue = Curve.Values[TimeIndex];
					FQuat Quat(RotationValue.X, RotationValue.Y, RotationValue.Z, RotationValue.W);
					FVector Euler = Quat.Euler();
					AnimationCurve->AddRotationValue(Curve.Timeline[TimeIndex], Euler, ERichCurveInterpMode::RCIM_Linear);
				}
			}
			else if (Path == TEXT("scale"))
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadNodeAnimation_AddScale);
				if (Curve.Timeline.Num() != Curve.Values.Num())
				{
					AddError(TEXT("LoadAllNodeAnimationCurves()"), FString::Printf(TEXT("Animation input/output mismatch (%d/%d) for scale on node %d"), Curve.Timeline.Num(), Curve.Values.Num(), Node.Index));
					return;
				}
				for (int32 TimeIndex = 0; TimeIndex < Curve.Timeline.Num(); TimeIndex++)
				{
					AnimationCurve->AddScaleValue(Curve.Timeline[TimeIndex], Curve.Values[TimeIndex], ERichCurveInterpMode::RCIM_Linear);
				}
			}
			bAnimationFound = true;
		};

	TArray<UglTFRuntimeAnimationCurve*> AnimationCurves;

	for (int32 JsonAnimationIndex = 0; JsonAnimationIndex < JsonAnimations->Num(); JsonAnimationIndex++)
	{
		TSharedPtr<FJsonObject> JsonAnimationObject = (*JsonAnimations)[JsonAnimationIndex]->AsObject();
		if (!JsonAnimationObject)
			continue;

		float Duration;
		FString Name;
		bAnimationFound = false;
		AnimationCurve = NewObject<UglTFRuntimeAnimationCurve>(GetTransientPackage(), NAME_None, RF_Public);
		AnimationCurve->SetDefaultValues(OriginalTransform.GetLocation(), OriginalTransform.Rotator().Euler(), OriginalTransform.GetScale3D());
		if (!LoadAnimation_Internal(JsonAnimationObject.ToSharedRef(), Duration, Name, Callback, [&](const FglTFRuntimeNode& Node) -> bool { return Node.Index == NodeIndex; }, {}))
		{
			continue;
		}

		// stop at the first found animation
		if (bAnimationFound)
		{
			AnimationCurve->glTFCurveAnimationIndex = JsonAnimationIndex;
			AnimationCurve->glTFCurveAnimationName = Name;
			AnimationCurve->glTFCurveAnimationDuration = Duration;
			AnimationCurve->BasisMatrix = SceneBasis;
			AnimationCurves.Add(AnimationCurve);
		}
	}

	AnimationCurvesCache.Add(NodeIndex, AnimationCurves);

	return AnimationCurves;
}
