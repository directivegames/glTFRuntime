// Copyright (C) 2020 - Directive Games Limited - All Rights Reserved

#include "glTFRuntimeParser.h"

#define SORT_ANIM_POINTS 0

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
			TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadNodeAnimation_Callback);
			Callback(Node, ChannelObject.TargetPath, Samplers[Sampler]);
		}
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

	const FTransform OriginalTransform = FTransform(SceneBasis * Node.Transform.ToMatrixWithScale() * SceneBasis.Inverse());
	TArray<UglTFRuntimeAnimationCurve*> AnimationCurves;

	for (int32 JsonAnimationIndex = 0; JsonAnimationIndex < JsonAnimations->Num(); JsonAnimationIndex++)
	{
		TSharedPtr<FJsonObject> JsonAnimationObject = (*JsonAnimations)[JsonAnimationIndex]->AsObject();
		if (!JsonAnimationObject)
			continue;

		float Duration;
		FString Name;
		auto bAnimationFound = false;
		auto AnimationCurve = NewObject<UglTFRuntimeAnimationCurve>(GetTransientPackage(), NAME_None, RF_Public);
		AnimationCurve->SetDefaultValues(OriginalTransform.GetLocation(), OriginalTransform.Rotator().Euler(), OriginalTransform.GetScale3D());

		const auto Callback = [&](const FglTFRuntimeNode& Node, const FString& Path, const FglTFRuntimeAnimationCurve& Curve)
		{			
			static TArray<FglTFRuntimeCurvePoint> Points;

			if (Path == TEXT("translation"))
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadNodeAnimation_AddTranslation);				
				if (Curve.Timeline.Num() != Curve.Values.Num())
				{
					AddError(TEXT("LoadAllNodeAnimationCurves()"), FString::Printf(TEXT("Animation input/output mismatch (%d/%d) for translation on node %d"), Curve.Timeline.Num(), Curve.Values.Num(), Node.Index));
					return;
				}

				Points.Reset(Curve.Timeline.Num());
				for (int32 TimeIndex = 0; TimeIndex < Curve.Timeline.Num(); TimeIndex++)
				{
					Points.Add({ Curve.Timeline[TimeIndex], Curve.Values[TimeIndex] * SceneScale });
				}
#if SORT_ANIM_POINTS
				Points.Sort([](const FglTFRuntimeCurvePoint& A, const FglTFRuntimeCurvePoint& B) { return A.Time < B.Time; });
#endif
				AnimationCurve->AddLocationValues(Points, ERichCurveInterpMode::RCIM_Linear);
			}
			else if (Path == TEXT("rotation"))
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadNodeAnimation_AddRotation);
				if (Curve.Timeline.Num() != Curve.Values.Num())
				{
					AddError(TEXT("LoadAllNodeAnimationCurves()"), FString::Printf(TEXT("Animation input/output mismatch (%d/%d) for rotation on node %d"), Curve.Timeline.Num(), Curve.Values.Num(), Node.Index));
					return;
				}

				Points.Reset(Curve.Timeline.Num());
				for (int32 TimeIndex = 0; TimeIndex < Curve.Timeline.Num(); TimeIndex++)
				{
					FVector4 RotationValue = Curve.Values[TimeIndex];
					FQuat Quat(RotationValue.X, RotationValue.Y, RotationValue.Z, RotationValue.W);
					FVector Euler = Quat.Euler();
					Points.Add({ Curve.Timeline[TimeIndex], Euler });
				}
#if SORT_ANIM_POINTS
				Points.Sort([](const FglTFRuntimeCurvePoint& A, const FglTFRuntimeCurvePoint& B) { return A.Time < B.Time; });
#endif
				AnimationCurve->AddRotationValues(Points, ERichCurveInterpMode::RCIM_Linear);
			}
			else if (Path == TEXT("scale"))
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(FglTFRuntimeParser::LoadNodeAnimation_AddScale);
				if (Curve.Timeline.Num() != Curve.Values.Num())
				{
					AddError(TEXT("LoadAllNodeAnimationCurves()"), FString::Printf(TEXT("Animation input/output mismatch (%d/%d) for scale on node %d"), Curve.Timeline.Num(), Curve.Values.Num(), Node.Index));
					return;
				}

				Points.Reset(Curve.Timeline.Num());
				for (int32 TimeIndex = 0; TimeIndex < Curve.Timeline.Num(); TimeIndex++)
				{
					Points.Add({ Curve.Timeline[TimeIndex], Curve.Values[TimeIndex] });
				}
#if SORT_ANIM_POINTS
				Points.Sort([](const FglTFRuntimeCurvePoint& A, const FglTFRuntimeCurvePoint& B) { return A.Time < B.Time; });
#endif
				AnimationCurve->AddScaleValues(Points, ERichCurveInterpMode::RCIM_Linear);
			}
			bAnimationFound = true;
		};

		const auto NodeFilter = [&](const FglTFRuntimeNode& Node) -> bool
		{
			return Node.Index == NodeIndex;
		};

		if (!LoadAnimation_Internal(JsonAnimationObject.ToSharedRef(), Duration, Name, Callback, NodeFilter, {}))
		{
			continue;
		}

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

static void UnwindRotationKeys(TArray<FRichCurveKey>& Keys)
{
	// See FRichCurve::AddKey
	for (auto Index = 1; Index < Keys.Num(); ++Index)
	{
		const auto& PreviousKey = Keys[Index - 1];
		auto& CurrentKey = Keys[Index];
		while (CurrentKey.Value - PreviousKey.Value > 180.0f)
		{
			CurrentKey.Value -= 360.0f;
		}
		while (CurrentKey.Value - PreviousKey.Value < -180.0f)
		{
			CurrentKey.Value += 360.0f;
		}
	}
}

static void PopulateCurves(FRichCurve(&Curves)[3], const TArray<FglTFRuntimeCurvePoint>& Points, ERichCurveInterpMode InterpolationMode, bool bUnwindRotation = false)
{
	const auto NumPoints = Points.Num();
	static TArray<FRichCurveKey> Keys[3];
	Keys[0].Reset(NumPoints);
	Keys[1].Reset(NumPoints);
	Keys[2].Reset(NumPoints);
	for (const auto& Point : Points)
	{
		Keys[0].Add(FRichCurveKey(Point.Time, Point.Value.X, 0.f, 0.f, InterpolationMode));
		Keys[1].Add(FRichCurveKey(Point.Time, Point.Value.Y, 0.f, 0.f, InterpolationMode));
		Keys[2].Add(FRichCurveKey(Point.Time, Point.Value.Z, 0.f, 0.f, InterpolationMode));
	}
	if (bUnwindRotation)
	{
		UnwindRotationKeys(Keys[0]);
		UnwindRotationKeys(Keys[1]);
		UnwindRotationKeys(Keys[2]);
	}
	Curves[0].SetKeys(Keys[0]);
	Curves[1].SetKeys(Keys[1]);
	Curves[2].SetKeys(Keys[2]);
}

void UglTFRuntimeAnimationCurve::AddLocationValues(const TArray<FglTFRuntimeCurvePoint>& Points, ERichCurveInterpMode InterpolationMode)
{
	PopulateCurves(LocationCurves, Points, InterpolationMode);
}

void UglTFRuntimeAnimationCurve::AddRotationValues(const TArray<FglTFRuntimeCurvePoint>& Points, ERichCurveInterpMode InterpolationMode)
{
	PopulateCurves(RotationCurves, Points, InterpolationMode, true);
}

void UglTFRuntimeAnimationCurve::AddScaleValues(const TArray<FglTFRuntimeCurvePoint>& Points, ERichCurveInterpMode InterpolationMode)
{
	PopulateCurves(ScaleCurves, Points, InterpolationMode);
}

float FglTFRuntimeParser::FindBestFrames(const TArray<float>& FramesTimes, float FrameDelta, int32 FrameNumber, int32& FirstIndex, int32& SecondIndex)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FindBestFrames);

	const auto NumFrames = FramesTimes.Num();
	const auto FirstFrameTime = FramesTimes[0];
	if (NumFrames >= 2)
	{
		const auto SourceFrameDelta = FramesTimes[1] - FirstFrameTime;
		if (FMath::IsNearlyEqual(SourceFrameDelta, FrameDelta) &&
			FMath::IsNearlyZero(FirstFrameTime) &&
			FramesTimes.IsValidIndex(FrameNumber))
		{
			FirstIndex = SecondIndex = FrameNumber;
			return 0.f;
		}
	}
	else
	{
		FirstIndex = SecondIndex = 0;
		return 0.f;
	}

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FindBestFrames_Slow);
		const auto WantedTime = FrameDelta * FrameNumber + FirstFrameTime;
		FirstIndex = INDEX_NONE;
		SecondIndex = INDEX_NONE;
		auto BeginIndex = 0;
		auto EndIndex = NumFrames - 1;
		while (BeginIndex <= EndIndex)
		{
			const auto MiddleIndex = (BeginIndex + EndIndex) / 2;
			if (MiddleIndex == BeginIndex || MiddleIndex == EndIndex)
			{
				FirstIndex = BeginIndex;
				SecondIndex = EndIndex;
				break;
			}
			const auto MiddleFrameTime = FramesTimes[MiddleIndex];
			if (FMath::IsNearlyEqual(MiddleFrameTime, WantedTime))
			{
				FirstIndex = MiddleIndex;
				SecondIndex = MiddleIndex;
				break;
			}
			else if (MiddleFrameTime < WantedTime)
			{
				BeginIndex = MiddleIndex;
			}
			else
			{
				EndIndex = MiddleIndex;
			}
		}

		check(FirstIndex != INDEX_NONE);
		if (FirstIndex == SecondIndex)
		{
			return 0.f;
		}
		else
		{
			const auto Alpha = (WantedTime - FramesTimes[FirstIndex]) / (FramesTimes[SecondIndex] - FramesTimes[FirstIndex]);
			if (Alpha < 0.f)
			{
				FirstIndex = SecondIndex = 0;
				return 0.f;
			}
			else if (Alpha > 1.f)
			{
				FirstIndex = SecondIndex = NumFrames - 1;
				return 0.f;
			}
			else
			{
				return Alpha;
			}
		}
	}
}
