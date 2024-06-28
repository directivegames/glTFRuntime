// Copyright 2020-2024, Roberto De Ioris.


#include "glTFRuntimeFunctionLibrary.h"
#include "Async/Async.h"
#include "HttpModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Runtime/Launch/Resources/Version.h"

#if 1 // WITH_DIRECTIVE
#include "Misc/Paths.h"
#include "glTFRuntimeAssetActor.h"
#include "Directive/glTFCacheSubsystem.h"
#endif


UglTFRuntimeAsset* UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilename(UObject* WorldContextObject, const FString& Filename, const bool bPathRelativeToContent, const FglTFRuntimeConfig& LoaderConfig)
{
#if 1 // WITH_DIRECTIVE
	const auto CacheKey = bPathRelativeToContent ? FPaths::Combine(FPaths::ProjectContentDir(), Filename) : Filename;
	if (auto CachedAsset = UglTFCacheSubsystem::GetCachedAsset(WorldContextObject, CacheKey))
	{
		return CachedAsset;
	}
#endif

	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		return nullptr;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

	// Annoying copy, but we do not want to remove the const
	FglTFRuntimeConfig OverrideConfig = LoaderConfig;

	if (bPathRelativeToContent)
	{
		OverrideConfig.bSearchContentDir = true;
	}

	if (!Asset->LoadFromFilename(Filename, OverrideConfig))
	{
		return nullptr;
	}

#if 1 // WITH_DIRECTIVE
	UglTFCacheSubsystem::CacheAsset(WorldContextObject, CacheKey, Asset);
#endif

	return Asset;
}

void UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFilenameAsync(UObject* WorldContextObject, const FString& Filename, const bool bPathRelativeToContent, const FglTFRuntimeConfig& LoaderConfig, const FglTFRuntimeHttpResponse& Completed)
{
#if 1 // WITH_DIRECTIVE
	const auto CacheKey = bPathRelativeToContent ? FPaths::Combine(FPaths::ProjectContentDir(), Filename) : Filename;
	if (auto CachedAsset = UglTFCacheSubsystem::GetCachedAsset(WorldContextObject, CacheKey))
	{
		AsyncTask(ENamedThreads::GameThread, [Completed, WeakCache = TWeakObjectPtr<UglTFRuntimeAsset>(CachedAsset)]()
			{
				Completed.ExecuteIfBound(WeakCache.Get());
			});
		return;
	}
#endif

	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		Completed.ExecuteIfBound(nullptr);
		return;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

	// Annoying copy, but we do not want to remove the const
	FglTFRuntimeConfig OverrideConfig = LoaderConfig;

	if (bPathRelativeToContent)
	{
		OverrideConfig.bSearchContentDir = true;
	}

#if 1 // WITH_DIRECTIVE
	Async(EAsyncExecution::Thread, [Filename, Asset, Completed, OverrideConfig, CacheKey, WeakContext = TWeakObjectPtr<UObject>(WorldContextObject)]()
#else
	Async(EAsyncExecution::Thread, [Filename, Asset, Completed, OverrideConfig]()
#endif
	{
		TSharedPtr<FglTFRuntimeParser> Parser = FglTFRuntimeParser::FromFilename(Filename, OverrideConfig);


		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Parser, Asset, Completed, WeakContext, CacheKey]()
		{
			if (Parser.IsValid() && Asset->SetParser(Parser.ToSharedRef()))
			{
#if 1 // WITH_DIRECTIVE
				UglTFCacheSubsystem::CacheAsset(WeakContext.Get(), CacheKey, Asset);
#endif
				Completed.ExecuteIfBound(Asset);
			}
			else
			{
				Completed.ExecuteIfBound(nullptr);
			}
		}, TStatId(), nullptr, ENamedThreads::GameThread);
		FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
	});
}

UglTFRuntimeAsset* UglTFRuntimeFunctionLibrary::glTFLoadAssetFromString(UObject* WorldContextObject, const FString& JsonData, const FglTFRuntimeConfig& LoaderConfig)
{
#if 1 // WITH_DIRECTIVE
	const auto CacheKey = UglTFCacheSubsystem::GetKeyHash(JsonData);
	if (auto CachedAsset = UglTFCacheSubsystem::GetCachedAsset(WorldContextObject, CacheKey))
	{
		return CachedAsset;
	}
#endif

	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		return nullptr;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

	if (!Asset->LoadFromString(JsonData, LoaderConfig))
	{
		return nullptr;
	}

#if 1 // WITH_DIRECTIVE
	UglTFCacheSubsystem::CacheAsset(WorldContextObject, CacheKey, Asset);
#endif

	return Asset;
}

UglTFRuntimeAsset* UglTFRuntimeFunctionLibrary::glTFLoadAssetFromBase64(UObject* WorldContextObject, const FString& Base64, const FglTFRuntimeConfig& LoaderConfig)
{
#if 1 // WITH_DIRECTIVE
	const auto CacheKey = UglTFCacheSubsystem::GetKeyHash(Base64);
	if (auto CachedAsset = UglTFCacheSubsystem::GetCachedAsset(WorldContextObject, CacheKey))
	{
		return CachedAsset;
	}
#endif

	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		return nullptr;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

	TArray<uint8> BytesBase64;

	if (!FBase64::Decode(Base64, BytesBase64))
	{
		return nullptr;
	}

	if (!Asset->LoadFromData(BytesBase64.GetData(), BytesBase64.Num(), LoaderConfig))
	{
		return nullptr;
	}

#if 1 // WITH_DIRECTIVE
	UglTFCacheSubsystem::CacheAsset(WorldContextObject, CacheKey, Asset);
#endif

	return Asset;
}

void UglTFRuntimeFunctionLibrary::glTFLoadAssetFromBase64Async(UObject* WorldContextObject, const FString& Base64, const FglTFRuntimeConfig& LoaderConfig, const FglTFRuntimeHttpResponse& Completed)
{
#if 1 // WITH_DIRECTIVE
	const auto CacheKey = UglTFCacheSubsystem::GetKeyHash(Base64);
	if (auto CachedAsset = UglTFCacheSubsystem::GetCachedAsset(WorldContextObject, CacheKey))
	{
		AsyncTask(ENamedThreads::GameThread, [Completed, WeakCache = TWeakObjectPtr<UglTFRuntimeAsset>(CachedAsset)]()
		{
			Completed.ExecuteIfBound(WeakCache.Get());
		});
		return;
	}
#endif

	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		Completed.ExecuteIfBound(nullptr);
		return;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

#if 1 // WITH_DIRECTIVE
	Async(EAsyncExecution::Thread, [Base64, Asset, LoaderConfig, Completed, CacheKey, WeakContext = TWeakObjectPtr<UObject>(WorldContextObject)]()
#else
	Async(EAsyncExecution::Thread, [Base64, Asset, LoaderConfig, Completed]()
#endif
		{
			TArray<uint8> BytesBase64;

			if (!FBase64::Decode(Base64, BytesBase64))
			{
				FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Completed]()
					{
						Completed.ExecuteIfBound(nullptr);
					}, TStatId(), nullptr, ENamedThreads::GameThread);
				FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
				return;
			}

			TSharedPtr<FglTFRuntimeParser> Parser = FglTFRuntimeParser::FromData(BytesBase64, LoaderConfig);

			FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Parser, Asset, Completed, CacheKey, WeakContext]()
				{
					if (Parser.IsValid() && Asset->SetParser(Parser.ToSharedRef()))
					{
#if 1 // WITH_DIRECTIVE
						UglTFCacheSubsystem::CacheAsset(WeakContext.Get(), CacheKey, Asset);
#endif
						Completed.ExecuteIfBound(Asset);
					}
					else
					{
						Completed.ExecuteIfBound(nullptr);
					}
				}, TStatId(), nullptr, ENamedThreads::GameThread);
			FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
		});
}

void UglTFRuntimeFunctionLibrary::glTFLoadAssetFromStringAsync(UObject* WorldContextObject, const FString& JsonData, const FglTFRuntimeConfig& LoaderConfig, const FglTFRuntimeHttpResponse& Completed)
{
#if 1 // WITH_DIRECTIVE
	const auto CacheKey = UglTFCacheSubsystem::GetKeyHash(JsonData);
	if (auto CachedAsset = UglTFCacheSubsystem::GetCachedAsset(WorldContextObject, CacheKey))
	{
		AsyncTask(ENamedThreads::GameThread, [Completed, WeakCache = TWeakObjectPtr<UglTFRuntimeAsset>(CachedAsset)]()
		{
			Completed.ExecuteIfBound(WeakCache.Get());
		});
		return;
	}
#endif

	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		Completed.ExecuteIfBound(nullptr);
		return;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

#if 1 // WITH_DIRECTIVE
	Async(EAsyncExecution::Thread, [JsonData, Asset, LoaderConfig, Completed, CacheKey, WeakContext = TWeakObjectPtr<UObject>(WorldContextObject)]()
#else
	Async(EAsyncExecution::Thread, [JsonData, Asset, LoaderConfig, Completed]()
#endif
		{
			TSharedPtr<FglTFRuntimeParser> Parser = FglTFRuntimeParser::FromString(JsonData, LoaderConfig);

			FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Parser, Asset, Completed, CacheKey, WeakContext]()
				{
					if (Parser.IsValid() && Asset->SetParser(Parser.ToSharedRef()))
					{
#if 1 // WITH_DIRECTIVE
						UglTFCacheSubsystem::CacheAsset(WeakContext.Get(), CacheKey, Asset);
#endif
						Completed.ExecuteIfBound(Asset);
					}
					else
					{
						Completed.ExecuteIfBound(nullptr);
					}
				}, TStatId(), nullptr, ENamedThreads::GameThread);
			FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
		});
}

UglTFRuntimeAsset* UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFileMap(const TMap<FString, FString>& FileMap, const FglTFRuntimeConfig& LoaderConfig)
{
	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		return nullptr;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

	TMap<FString, TArray64<uint8>> Map;

	for (const TPair<FString, FString>& Pair : FileMap)
	{
		TArray64<uint8> Data;
		if (FFileHelper::LoadFileToArray(Data, *Pair.Value))
		{
			Map.Add(Pair.Key, MoveTemp(Data));
		}
	}

	TSharedPtr<FglTFRuntimeParser> Parser = FglTFRuntimeParser::FromMap(Map, LoaderConfig);

	if (Parser.IsValid() && Asset->SetParser(Parser.ToSharedRef()))
	{
		return Asset;
	}

	return nullptr;
}

void UglTFRuntimeFunctionLibrary::glTFLoadAssetFromFileMapAsync(const TMap<FString, FString>& FileMap, const FglTFRuntimeConfig& LoaderConfig, const FglTFRuntimeHttpResponse& Completed)
{
	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		Completed.ExecuteIfBound(nullptr);
		return;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

	Async(EAsyncExecution::Thread, [FileMap, Asset, LoaderConfig, Completed]()
		{
			TMap<FString, TArray64<uint8>> Map;

			for (const TPair<FString, FString>& Pair : FileMap)
			{
				TArray64<uint8> Data;
				if (FFileHelper::LoadFileToArray(Data, *Pair.Value))
				{
					Map.Add(Pair.Key, MoveTemp(Data));
				}
			}

			TSharedPtr<FglTFRuntimeParser> Parser = FglTFRuntimeParser::FromMap(Map, LoaderConfig);

			FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Parser, Asset, Completed]()
				{
					if (Parser.IsValid() && Asset->SetParser(Parser.ToSharedRef()))
					{
						Completed.ExecuteIfBound(Asset);
					}
					else
					{
						Completed.ExecuteIfBound(nullptr);
					}
				}, TStatId(), nullptr, ENamedThreads::GameThread);
			FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
		});
}

void UglTFRuntimeFunctionLibrary::glTFLoadAssetFromUrl(UObject* WorldContextObject, const FString& Url, const TMap<FString, FString>& Headers, FglTFRuntimeHttpResponse Completed, const FglTFRuntimeConfig& LoaderConfig)
{
#if 1 // WITH_DIRECTIVE
	if (auto CachedAsset = UglTFCacheSubsystem::GetCachedAsset(WorldContextObject, Url))
	{
		AsyncTask(ENamedThreads::GameThread, [Completed, WeakCache = TWeakObjectPtr<UglTFRuntimeAsset>(CachedAsset)]()
			{
				Completed.ExecuteIfBound(WeakCache.Get());
			});
		return;
}
#endif

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 25
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#else
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
	HttpRequest->SetURL(Url);

#if 1 // WITH_DIRECTIVE
	HttpRequest->SetVerb(TEXT("GET"));
#endif

	for (TPair<FString, FString> Header : Headers)
	{
		HttpRequest->AppendToHeader(Header.Key, Header.Value);
	}

	float StartTime = FPlatformTime::Seconds();

#if 1 // WITH_DIRECTIVE
	HttpRequest->OnProcessRequestComplete().BindLambda([StartTime, Url, WeakContext = TWeakObjectPtr<UObject>(WorldContextObject)](FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bSuccess, FglTFRuntimeHttpResponse Completed, const FglTFRuntimeConfig& LoaderConfig)
#else
	HttpRequest->OnProcessRequestComplete().BindLambda([StartTime](FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bSuccess, FglTFRuntimeHttpResponse Completed, const FglTFRuntimeConfig& LoaderConfig)
#endif
		{
			UglTFRuntimeAsset* Asset = nullptr;
			if (bSuccess && !IsGarbageCollecting())
			{
#if 1 // WITH_DIRECTIVE
				Asset = glTFLoadAssetFromData(WeakContext.Get(), ResponsePtr->GetContent(), LoaderConfig);
				UglTFCacheSubsystem::CacheAsset(WeakContext.Get(), Url, Asset);
#else
				Asset = glTFLoadAssetFromData(ResponsePtr->GetContent(), LoaderConfig);
#endif
				if (Asset)
				{
					Asset->GetParser()->SetDownloadTime(FPlatformTime::Seconds() - StartTime);
				}
			}
			Completed.ExecuteIfBound(Asset);
		}, Completed, LoaderConfig);

	HttpRequest->ProcessRequest();
}

void UglTFRuntimeFunctionLibrary::glTFLoadAssetFromUrlWithProgress(UObject* WorldContextObject, const FString& Url, const TMap<FString, FString>& Headers, FglTFRuntimeHttpResponse Completed, FglTFRuntimeHttpProgress Progress, const FglTFRuntimeConfig& LoaderConfig)
{
#if 1 // WITH_DIRECTIVE
	if (auto CachedAsset = UglTFCacheSubsystem::GetCachedAsset(WorldContextObject, Url))
	{
		AsyncTask(ENamedThreads::GameThread, [Completed, WeakCache = TWeakObjectPtr<UglTFRuntimeAsset>(CachedAsset)]()
			{
				Completed.ExecuteIfBound(WeakCache.Get());
			});
		return;
}
#endif

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 25
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
#else
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
#endif
	HttpRequest->SetURL(Url);
	for (TPair<FString, FString> Header : Headers)
	{
		HttpRequest->AppendToHeader(Header.Key, Header.Value);
	}

	float StartTime = FPlatformTime::Seconds();

#if 1 // WITH_DIRECTIVE
	HttpRequest->OnProcessRequestComplete().BindLambda([StartTime, Url, WeakContext = TWeakObjectPtr<UObject>(WorldContextObject)](FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bSuccess, FglTFRuntimeHttpResponse Completed, const FglTFRuntimeConfig& LoaderConfig)
#else
	HttpRequest->OnProcessRequestComplete().BindLambda([StartTime](FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bSuccess, FglTFRuntimeHttpResponse Completed, const FglTFRuntimeConfig& LoaderConfig)
#endif
		{
			UglTFRuntimeAsset* Asset = nullptr;
			if (bSuccess && !IsGarbageCollecting())
			{
#if 1 // WITH_DIRECTIVE
				Asset = glTFLoadAssetFromData(WeakContext.Get(), ResponsePtr->GetContent(), LoaderConfig);
				UglTFCacheSubsystem::CacheAsset(WeakContext.Get(), Url, Asset);
#else
				Asset = glTFLoadAssetFromData(ResponsePtr->GetContent(), LoaderConfig);
#endif
				if (Asset)
				{
					Asset->GetParser()->SetDownloadTime(FPlatformTime::Seconds() - StartTime);
				}
			}
			Completed.ExecuteIfBound(Asset);
		}, Completed, LoaderConfig);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
	HttpRequest->OnRequestProgress64().BindLambda([](FHttpRequestPtr RequestPtr, uint64 BytesSent, uint64 BytesReceived, FglTFRuntimeHttpProgress Progress, const FglTFRuntimeConfig& LoaderConfig)
#else
	HttpRequest->OnRequestProgress().BindLambda([](FHttpRequestPtr RequestPtr, int32 BytesSent, int32 BytesReceived, FglTFRuntimeHttpProgress Progress, const FglTFRuntimeConfig& LoaderConfig)
#endif
		{
			int32 ContentLength = 0;
			if (RequestPtr->GetResponse().IsValid())
			{
				ContentLength = RequestPtr->GetResponse()->GetContentLength();
			}
			Progress.ExecuteIfBound(LoaderConfig, BytesReceived, ContentLength);
		}, Progress, LoaderConfig);

	HttpRequest->ProcessRequest();
}

UglTFRuntimeAsset* UglTFRuntimeFunctionLibrary::glTFLoadAssetFromData(UObject* WorldContextObject, const TArray<uint8>& Data, const FglTFRuntimeConfig& LoaderConfig)
{
#if 1 // WITH_DIRECTIVE
	const auto CacheKey = UglTFCacheSubsystem::GetKeyHash(Data);
	if (auto CachedAsset = UglTFCacheSubsystem::GetCachedAsset(WorldContextObject, CacheKey))
	{
		return CachedAsset;
	}
#endif

	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		return nullptr;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

	if (!Asset->LoadFromData(Data.GetData(), Data.Num(), LoaderConfig))
	{
		return nullptr;
	}

#if 1 // WITH_DIRECTIVE
	UglTFCacheSubsystem::CacheAsset(WorldContextObject, CacheKey, Asset);
#endif

	return Asset;
}

bool UglTFRuntimeFunctionLibrary::glTFLoadAssetFromClipboard(UObject* WorldContextObject, FglTFRuntimeHttpResponse Completed, FString& ClipboardContent, const FglTFRuntimeConfig& LoaderConfig)
{

	FString Url;
	FPlatformApplicationMisc::ClipboardPaste(Url);

	if (Url.IsEmpty())
	{
		return false;
	}

	// escaped?
	if (Url.StartsWith("\"") && Url.EndsWith("\""))
	{
		Url = Url.RightChop(1).LeftChop(1);
	}

	ClipboardContent = Url;

#if 1 // WITH_DIRECTIVE
	if (Url.Contains("://"))
	{
		glTFLoadAssetFromUrl(WorldContextObject, Url, {}, Completed, LoaderConfig);
		return true;
	}

	UglTFRuntimeAsset* Asset = glTFLoadAssetFromFilename(WorldContextObject, Url, false, LoaderConfig);
#else
	if (Url.Contains("://"))
	{
		glTFLoadAssetFromUrl(Url, {}, Completed, LoaderConfig);
		return true;
	}

	UglTFRuntimeAsset* Asset = glTFLoadAssetFromFilename(Url, false, LoaderConfig);
#endif
	Completed.ExecuteIfBound(Asset);

	return Asset != nullptr;
}

TArray<FglTFRuntimePathItem> UglTFRuntimeFunctionLibrary::glTFRuntimePathItemArrayFromJSONPath(const FString& JSONPath)
{
	TArray<FglTFRuntimePathItem> Paths;
	TArray<FString> Keys;
	JSONPath.ParseIntoArray(Keys, TEXT("."));

	for (const FString& Key : Keys)
	{
		FString PathKey = Key;
		int32 PathIndex = -1;

		int32 SquareBracketStart = 0;
		int32 SquareBracketEnd = 0;
		if (Key.FindChar('[', SquareBracketStart))
		{
			if (Key.FindChar(']', SquareBracketEnd))
			{
				if (SquareBracketEnd > SquareBracketStart)
				{
					const FString KeyIndex = Key.Mid(SquareBracketStart + 1, SquareBracketEnd - SquareBracketStart);
					PathIndex = FCString::Atoi(*KeyIndex);
					PathKey = Key.Left(SquareBracketStart);
				}
			}
		}

		FglTFRuntimePathItem PathItem;
		PathItem.Path = PathKey;
		PathItem.Index = PathIndex;
		Paths.Add(PathItem);
	}

	return Paths;
}

bool UglTFRuntimeFunctionLibrary::GetIndicesAsBytesFromglTFRuntimeLODPrimitive(const FglTFRuntimeMeshLOD& RuntimeLOD, const int32 PrimitiveIndex, TArray<uint8>& Bytes)
{
	if (!RuntimeLOD.Primitives.IsValidIndex(PrimitiveIndex))
	{
		return false;
	}

	const FglTFRuntimePrimitive& Primitive = RuntimeLOD.Primitives[PrimitiveIndex];

	Bytes.AddUninitialized(Primitive.Indices.Num() * sizeof(uint32));
	FMemory::Memcpy(Bytes.GetData(), Primitive.Indices.GetData(), Primitive.Indices.Num() * sizeof(uint32));
	return true;
}

bool UglTFRuntimeFunctionLibrary::GetPositionsAsBytesFromglTFRuntimeLODPrimitive(const FglTFRuntimeMeshLOD& RuntimeLOD, const int32 PrimitiveIndex, TArray<uint8>& Bytes)
{
	if (!RuntimeLOD.Primitives.IsValidIndex(PrimitiveIndex))
	{
		return false;
	}

	const FglTFRuntimePrimitive& Primitive = RuntimeLOD.Primitives[PrimitiveIndex];
	Bytes.Reserve(Primitive.Positions.Num() * sizeof(float) * 3);
	for (const FVector& Position : Primitive.Positions)
	{
		float X = static_cast<float>(Position.X);
		float Y = static_cast<float>(Position.Y);
		float Z = static_cast<float>(Position.Z);
		Bytes.Append(reinterpret_cast<const uint8*>(&X), sizeof(float));
		Bytes.Append(reinterpret_cast<const uint8*>(&Y), sizeof(float));
		Bytes.Append(reinterpret_cast<const uint8*>(&Z), sizeof(float));
	}
	return true;
}

bool UglTFRuntimeFunctionLibrary::GetNormalsAsBytesFromglTFRuntimeLODPrimitive(const FglTFRuntimeMeshLOD& RuntimeLOD, const int32 PrimitiveIndex, TArray<uint8>& Bytes)
{
	if (!RuntimeLOD.Primitives.IsValidIndex(PrimitiveIndex))
	{
		return false;
	}

	const FglTFRuntimePrimitive& Primitive = RuntimeLOD.Primitives[PrimitiveIndex];
	Bytes.Reserve(Primitive.Positions.Num() * sizeof(float) * 3);
	for (const FVector& Normal : Primitive.Normals)
	{
		float X = static_cast<float>(Normal.X);
		float Y = static_cast<float>(Normal.Y);
		float Z = static_cast<float>(Normal.Z);
		Bytes.Append(reinterpret_cast<const uint8*>(&X), sizeof(float));
		Bytes.Append(reinterpret_cast<const uint8*>(&Y), sizeof(float));
		Bytes.Append(reinterpret_cast<const uint8*>(&Z), sizeof(float));
	}
	return true;
}

FglTFRuntimeMeshLOD UglTFRuntimeFunctionLibrary::glTFMergeRuntimeLODs(const TArray<FglTFRuntimeMeshLOD>& RuntimeLODs)
{
	FglTFRuntimeMeshLOD NewRuntimeLOD;

	for (const FglTFRuntimeMeshLOD& RuntimeLOD : RuntimeLODs)
	{
		NewRuntimeLOD.Primitives.Append(RuntimeLOD.Primitives);
		NewRuntimeLOD.AdditionalTransforms.Append(RuntimeLOD.AdditionalTransforms);
		if (NewRuntimeLOD.Skeleton.Num() == 0)
		{
			NewRuntimeLOD.Skeleton = RuntimeLOD.Skeleton;
		}
		if (!NewRuntimeLOD.bHasNormals)
		{
			NewRuntimeLOD.bHasNormals = RuntimeLOD.bHasNormals;
		}
		if (NewRuntimeLOD.bHasTangents)
		{
			NewRuntimeLOD.bHasTangents = RuntimeLOD.bHasTangents;
		}
		if (!NewRuntimeLOD.bHasUV)
		{
			NewRuntimeLOD.bHasUV = RuntimeLOD.bHasUV;
		}
		if (!NewRuntimeLOD.bHasVertexColors)
		{
			NewRuntimeLOD.bHasVertexColors = RuntimeLOD.bHasVertexColors;
		}
	}

	return NewRuntimeLOD;
}

void UglTFRuntimeFunctionLibrary::glTFLoadAssetFromCommand(const FString& Command, const FString& Arguments, const FString& WorkingDirectory, const FglTFRuntimeCommandResponse& Completed, const FglTFRuntimeConfig& LoaderConfig, const int32 ExpectedExitCode)
{
	UglTFRuntimeAsset* Asset = NewObject<UglTFRuntimeAsset>();
	if (!Asset)
	{
		Completed.ExecuteIfBound(nullptr, -1, "");
		return;
	}

	Asset->RuntimeContextObject = LoaderConfig.RuntimeContextObject;
	Asset->RuntimeContextString = LoaderConfig.RuntimeContextString;

	Async(EAsyncExecution::Thread, [Command, Arguments, WorkingDirectory, Asset, LoaderConfig, Completed, ExpectedExitCode]()
		{
			TArray<uint8> Bytes;

			void* ReadPipe = nullptr;
			void* WritePipe = nullptr;

			if (!FPlatformProcess::CreatePipe(ReadPipe, WritePipe))
			{
				FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Completed]()
					{
						Completed.ExecuteIfBound(nullptr, -1, "Unable to create process pipe");
					}, TStatId(), nullptr, ENamedThreads::GameThread);
				FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
				return;
			}

			FProcHandle ProcHandle = FPlatformProcess::CreateProc(
				*Command,
				*Arguments,
				true,
				true,
				true,
				nullptr,
				0,
				WorkingDirectory.IsEmpty() ? nullptr : *WorkingDirectory,
				WritePipe,
				ReadPipe);

			if (!ProcHandle.IsValid())
			{
				FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
				FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Completed]()
					{
						Completed.ExecuteIfBound(nullptr, -1, "Unable to launch process");
					}, TStatId(), nullptr, ENamedThreads::GameThread);
				FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
				return;
			}

			while (FPlatformProcess::IsProcRunning(ProcHandle))
			{
				TArray<uint8> PipeChunk;
				if (FPlatformProcess::ReadPipeToArray(ReadPipe, PipeChunk))
				{
					Bytes.Append(PipeChunk);
				}
			}

			int32 ReturnCode = 0;
			FPlatformProcess::GetProcReturnCode(ProcHandle, &ReturnCode);

			FPlatformProcess::CloseProc(ProcHandle);
			FPlatformProcess::ClosePipe(ReadPipe, WritePipe);

			if (ReturnCode != ExpectedExitCode)
			{
				FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Completed, ReturnCode, &Bytes]()
					{
						Completed.ExecuteIfBound(nullptr, ReturnCode, FString::FromBlob(Bytes.GetData(), Bytes.Num()));
					}, TStatId(), nullptr, ENamedThreads::GameThread);
				FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
				return;
			}

			TSharedPtr<FglTFRuntimeParser> Parser = FglTFRuntimeParser::FromData(Bytes, LoaderConfig);

			FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Parser, Asset, Completed, ReturnCode]()
				{
					if (Parser.IsValid() && Asset->SetParser(Parser.ToSharedRef()))
					{
						Completed.ExecuteIfBound(Asset, ReturnCode, "");
					}
					else
					{
						Completed.ExecuteIfBound(nullptr, ReturnCode, "Unable to parse command output");
					}
				}, TStatId(), nullptr, ENamedThreads::GameThread);
			FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
		});

}

#if 1 // WITH_DIRECTIVE
AActor* UglTFRuntimeFunctionLibrary::glTFSpawnAssetOnActor(const UObject* WorldContextObject, UglTFRuntimeAsset* Asset, AActor* Actor, const FTransform& RelativeTransform,
	const FglTFRuntimeStaticMeshConfig& StaticMeshConfig,
	const FglTFRuntimeSkeletalMeshConfig& SkeletalMeshConfig,
	const FglTFRuntimeSkeletalAnimationConfig& SkeletalAnimationConfig)
{
	return glTFSpawnAssetOnActor2(WorldContextObject, Asset, Actor, RelativeTransform, StaticMeshConfig, SkeletalMeshConfig, SkeletalAnimationConfig);
}

AActor* UglTFRuntimeFunctionLibrary::glTFSpawnAssetOnActor2(const UObject* WorldContextObject, UglTFRuntimeAsset* Asset, AActor* Actor, const FTransform& RelativeTransform,
	const FglTFRuntimeStaticMeshConfig& StaticMeshConfig,
	const FglTFRuntimeSkeletalMeshConfig& SkeletalMeshConfig,
	const FglTFRuntimeSkeletalAnimationConfig& SkeletalAnimationConfig,
	TFunction<void(AglTFRuntimeAssetActor*)> PreSpawnConfiguration)
{
	if (ensure(WorldContextObject && Asset))
	{
		auto World = WorldContextObject->GetWorld();
		if (ensure(World))
		{
			auto TempActor = World->SpawnActorDeferred<AglTFRuntimeAssetActor>(AglTFRuntimeAssetActor::StaticClass(), FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			TempActor->Asset = Asset;
			if (!Actor)
			{
				Actor = TempActor;
			}

			if (RelativeTransform.Equals(FTransform::Identity))
			{
				TempActor->DelegateRootComponent = Actor->GetRootComponent();
			}
			else
			{
				auto SceneComponent = NewObject<USceneComponent>(Actor);
				SceneComponent->SetupAttachment(Actor->GetRootComponent());
				SceneComponent->SetRelativeTransform(RelativeTransform);
				SceneComponent->RegisterComponent();
				Actor->AddInstanceComponent(SceneComponent);
				TempActor->DelegateRootComponent = SceneComponent;
			}
			TempActor->StaticMeshConfig = StaticMeshConfig;
			TempActor->SkeletalMeshConfig = SkeletalMeshConfig;
			TempActor->SkeletalAnimationConfig = SkeletalAnimationConfig;
			if (PreSpawnConfiguration)
			{
				PreSpawnConfiguration(TempActor);
			}
			TempActor->FinishSpawning(FTransform::Identity);

			if (Actor != TempActor)
			{
				TempActor->SetLifeSpan(0.1f);
			}

			return Actor;
		}
	}
	return nullptr;
}
#endif
