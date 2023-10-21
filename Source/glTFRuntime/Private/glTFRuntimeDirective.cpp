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

		{
			TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadAnimation_Internal_Callback);
			Callback(Node, ChannelObject.TargetPath, Samplers[Sampler]);
		}
	}

	return true;
}
