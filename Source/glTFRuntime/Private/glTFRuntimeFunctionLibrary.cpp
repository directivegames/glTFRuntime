// Copyright 2020-2023, Roberto De Ioris.


#include "glTFRuntimeFunctionLibrary.h"
#include "Async/Async.h"
#include "HttpModule.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
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
	if (Asset)
	{
		UglTFCacheSubsystem::CacheAsset(WorldContextObject, CacheKey, Asset);
	}
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


			FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([Parser, Asset, Completed, WeakContext, Filename, CacheKey]()
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
	if (Asset)
	{
		UglTFCacheSubsystem::CacheAsset(WorldContextObject, CacheKey, Asset);
	}
#endif

	return Asset;
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
			if (bSuccess)
			{
#if 1 // WITH_DIRECTIVE
				Asset = glTFLoadAssetFromData(WeakContext.Get(), ResponsePtr->GetContent(), LoaderConfig);
				if (Asset)
				{
					Asset->GetParser()->SetDownloadTime(FPlatformTime::Seconds() - StartTime);
					UglTFCacheSubsystem::CacheAsset(WeakContext.Get(), Url, Asset);
				}
#else
				Asset = glTFLoadAssetFromData(ResponsePtr->GetContent(), LoaderConfig);
				if (Asset)
				{
					Asset->GetParser()->SetDownloadTime(FPlatformTime::Seconds() - StartTime);
				}
#endif				
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

#if 1 // WITH_DIRECTIVE
	HttpRequest->OnProcessRequestComplete().BindLambda([Url, WeakContext = TWeakObjectPtr<UObject>(WorldContextObject)](FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bSuccess, FglTFRuntimeHttpResponse Completed, const FglTFRuntimeConfig& LoaderConfig)
#else
	HttpRequest->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr RequestPtr, FHttpResponsePtr ResponsePtr, bool bSuccess, FglTFRuntimeHttpResponse Completed, const FglTFRuntimeConfig& LoaderConfig)
#endif
		{
			UglTFRuntimeAsset* Asset = nullptr;
			if (bSuccess)
			{
#if 1 // WITH_DIRECTIVE
				Asset = glTFLoadAssetFromData(WeakContext.Get(), ResponsePtr->GetContent(), LoaderConfig);
				if (Asset)
				{
					UglTFCacheSubsystem::CacheAsset(WeakContext.Get(), Url, Asset);
				}
#else
				Asset = glTFLoadAssetFromData(ResponsePtr->GetContent(), LoaderConfig);
#endif
			}
			Completed.ExecuteIfBound(Asset);
		}, Completed, LoaderConfig);

	HttpRequest->OnRequestProgress().BindLambda([](FHttpRequestPtr RequestPtr, int32 BytesSent, int32 BytesReceived, FglTFRuntimeHttpProgress Progress, const FglTFRuntimeConfig& LoaderConfig)
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
	if (Asset)
	{
		UglTFCacheSubsystem::CacheAsset(WorldContextObject, CacheKey, Asset);
	}
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
					const FString KeyIndex = Key.Mid(SquareBracketStart + 1, SquareBracketEnd - SquareBracketEnd);
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

#if 1 // WITH_DIRECTIVE
AActor* UglTFRuntimeFunctionLibrary::glTFSpawnAssetOnActor(const UObject* WorldContextObject, UglTFRuntimeAsset* Asset, AActor* Actor, const FTransform& RelativeTransform,
														const FglTFRuntimeStaticMeshConfig& StaticMeshConfig,
														const FglTFRuntimeSkeletalMeshConfig& SkeletalMeshConfig,
														const FglTFRuntimeSkeletalAnimationConfig& SkeletalAnimationConfig)
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
				TempActor->DelegateRootComponent = SceneComponent;
			}
			TempActor->StaticMeshConfig = StaticMeshConfig;
			TempActor->SkeletalMeshConfig = SkeletalMeshConfig;
			TempActor->SkeletalAnimationConfig = SkeletalAnimationConfig;
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
