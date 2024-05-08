// Copyright (C) 2023 - Directive Games Limited - All Rights Reserved


#include "Directive/glTFCacheSubsystem.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"
#include "Misc/SecureHash.h"


FString UglTFCacheSubsystem::GetKeyHash(const FString& LongString)
{
	return FMD5::HashBytes((const uint8*)*LongString, LongString.Len() * sizeof(TCHAR));
}

FString UglTFCacheSubsystem::GetKeyHash(const TArray<uint8>& Data)
{
	return FMD5::HashBytes(Data.GetData(), Data.Num());
}

void UglTFCacheSubsystem::CacheAsset(const FString& Key, UglTFRuntimeAsset* Asset)
{
	if (ensure(Asset))
	{
		CachedAssets.Add(Key, Asset);
	}
}

UglTFRuntimeAsset* UglTFCacheSubsystem::GetCachedAsset(const FString& Key) const
{
	auto Asset = CachedAssets.Find(Key);
	return Asset ? Asset->Get() : nullptr;
}


void UglTFCacheSubsystem::CacheAsset(UObject* WorldContextObject, const FString& Key, UglTFRuntimeAsset* Asset)
{
	if (!Asset)
	{
		return;
	}

	if (auto Sub = Cast<UglTFCacheSubsystem>(USubsystemBlueprintLibrary::GetGameInstanceSubsystem(WorldContextObject, UglTFCacheSubsystem::StaticClass())))
	{
		Sub->CacheAsset(Key, Asset);
	}
}

UglTFRuntimeAsset* UglTFCacheSubsystem::GetCachedAsset(UObject* WorldContextObject, const FString& Key)
{
	if (auto Sub = Cast<UglTFCacheSubsystem>(USubsystemBlueprintLibrary::GetGameInstanceSubsystem(WorldContextObject, UglTFCacheSubsystem::StaticClass())))
	{
		return Sub->GetCachedAsset(Key);
	}

	return nullptr;
}
