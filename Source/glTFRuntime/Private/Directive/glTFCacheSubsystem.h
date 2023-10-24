// Copyright (C) 2023 - Directive Games Limited - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "glTFRuntimeAsset.h"

#include "glTFCacheSubsystem.generated.h"


UCLASS()
class UglTFCacheSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static FString GetKeyHash(const FString& LongString);
	static FString GetKeyHash(const TArray<uint8>& Data);

	static void CacheAsset(UObject* WorldContextObject, const FString& Key, UglTFRuntimeAsset* Asset);
	static UglTFRuntimeAsset* GetCachedAsset(UObject* WorldContextObject, const FString& Key);	


	void CacheAsset(const FString& Key, UglTFRuntimeAsset* Asset);
	UglTFRuntimeAsset* GetCachedAsset(const FString& Key) const;

private:
	TMap<FString, TWeakObjectPtr<UglTFRuntimeAsset>> CachedAssets;
};
