// Copyright 2020, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "glTFRuntimeAsset.h"
#include "glTFRuntimeFunctionLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FglTFRuntimeHttpResponse, UglTFRuntimeAsset*, Asset);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FglTFRuntimeHttpProgress, const FglTFRuntimeConfig&, LoaderConfig, int32, BytesProcessed, int32, TotalBytes);

/**
 * 
 */
UCLASS()
class GLTFRUNTIME_API UglTFRuntimeFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(WorldContext = "WorldContextObject", DisplayName="glTF Load Asset from Filename", AutoCreateRefTerm = "LoaderConfig"), Category="glTFRuntime")
	static UglTFRuntimeAsset* glTFLoadAssetFromFilename(UObject* WorldContextObject, const FString& Filename, const bool bPathRelativeToContent, const FglTFRuntimeConfig& LoaderConfig);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "glTF Load Asset from String", AutoCreateRefTerm = "LoaderConfig"), Category = "glTFRuntime")
	static UglTFRuntimeAsset* glTFLoadAssetFromString(UObject* WorldContextObject, const FString& JsonData, const FglTFRuntimeConfig& LoaderConfig);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "glTF Load Asset from Url", AutoCreateRefTerm = "LoaderConfig, Headers"), Category = "glTFRuntime")
	static void glTFLoadAssetFromUrl(UObject* WorldContextObject, const FString& Url, const TMap<FString, FString>& Headers, FglTFRuntimeHttpResponse Completed, const FglTFRuntimeConfig& LoaderConfig);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "glTF Load Asset from Url with Progress", AutoCreateRefTerm = "LoaderConfig, Headers"), Category = "glTFRuntime")
	static void glTFLoadAssetFromUrlWithProgress(UObject* WorldContextObject, const FString& Url, const TMap<FString, FString>& Headers, FglTFRuntimeHttpResponse Completed, FglTFRuntimeHttpProgress Progress, const FglTFRuntimeConfig& LoaderConfig);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "glTF Load Asset from Data", AutoCreateRefTerm = "LoaderConfig"), Category = "glTFRuntime")
	static UglTFRuntimeAsset* glTFLoadAssetFromData(UObject* WorldContextObject, const TArray<uint8>& Data, const FglTFRuntimeConfig& LoaderConfig);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "glTF Load Asset from Clipboard", AutoCreateRefTerm = "LoaderConfig"), Category = "glTFRuntime")
	static bool glTFLoadAssetFromClipboard(UObject* WorldContextObject, FglTFRuntimeHttpResponse Completed, FString& ClipboardContent, const FglTFRuntimeConfig& LoaderConfig);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "glTF Load Asset from Filename Async", AutoCreateRefTerm = "LoaderConfig"), Category = "glTFRuntime")
	static void glTFLoadAssetFromFilenameAsync(UObject* WorldContextObject, const FString& Filename, const bool bPathRelativeToContent, const FglTFRuntimeConfig& LoaderConfig, const FglTFRuntimeHttpResponse& Completed);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Make glTFRuntime PathItem Array from JSONPath String"), Category = "glTFRuntime")
	static TArray<FglTFRuntimePathItem> glTFRuntimePathItemArrayFromJSONPath(const FString& JSONPath);

#if 1 // WITH_DIRECTIVE
	/*
	* Spawn the asset contained in 'Asset' onto 'Actor' at the specified 'RelativeTransform'.
	* If the actor is null, a new 'glTFRuntimeAssetActor' will be created.
	*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "glTF Spawn Asset On Actor"), Category = "glTFRuntime")
	static AActor* glTFSpawnAssetOnActor(const UObject* WorldContextObject, UglTFRuntimeAsset* Asset, AActor* Actor, const FTransform& RelativeTransform,
									  const FglTFRuntimeStaticMeshConfig& StaticMeshConfig,
									  const FglTFRuntimeSkeletalMeshConfig& SkeletalMeshConfig,
									  const FglTFRuntimeSkeletalAnimationConfig& SkeletalAnimationConfig);
#endif
};
