// Copyright 2020, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "glTFRuntimeAsset.h"
#include "glTFRuntimeAssetActor.generated.h"

UCLASS()
class GLTFRUNTIME_API AglTFRuntimeAssetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AglTFRuntimeAssetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void ProcessNode(USceneComponent* NodeParentComponent, const FName SocketName, FglTFRuntimeNode& Node);

#if 0 // WITH_DIRECTIVE
	TMap<USceneComponent*, float>  CurveBasedAnimationsTimeTracker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "glTFRuntime")
	TSet<FString> DiscoveredCurveAnimationsNames;

	TMap<USceneComponent*, TMap<FString, UglTFRuntimeAnimationCurve*>> DiscoveredCurveAnimations;
#endif

#if 1 // WITH_DIRECTIVE
	int32 NameIndex = 1000;

	template<typename T>
	FName GetSafeNodeName(const FglTFRuntimeNode& Node)
	{
		if (Guid.IsValid())
		{
			FString BaseName = Node.Name + TEXT("_") + Guid.ToString();
			NameIndex += 1;

			return FName(BaseName, NameIndex);
		}
		
		return MakeUniqueObjectName(this, T::StaticClass(), *Node.Name);
	}
#else
	template<typename T>
	FName GetSafeNodeName(const FglTFRuntimeNode& Node)
	{
		return MakeUniqueObjectName(this, T::StaticClass(), *Node.Name);
	}
#endif

	TMap<USceneComponent*, FName> SocketMapping;
	TArray<USkeletalMeshComponent*> DiscoveredSkeletalMeshComponents;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	UglTFRuntimeAsset* Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	FglTFRuntimeStaticMeshConfig StaticMeshConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	FglTFRuntimeSkeletalMeshConfig SkeletalMeshConfig;

#if 1 // WITH_DIRECTIVE
	// If specified, newly created components will be attached to the delegate root
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	TObjectPtr<USceneComponent> DelegateRootComponent;

	FGuid Guid;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	FglTFRuntimeSkeletalAnimationConfig SkeletalAnimationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	FglTFRuntimeLightConfig LightConfig;

#if 0 // WITH_DIRECTIVE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "glTFRuntime")
	TMap<USceneComponent*, UglTFRuntimeAnimationCurve*> CurveBasedAnimations;
#endif

	UFUNCTION(BlueprintNativeEvent, Category = "glTFRuntime", meta = (DisplayName = "On StaticMeshComponent Created"))
	void ReceiveOnStaticMeshComponentCreated(UStaticMeshComponent* StaticMeshComponent, const FglTFRuntimeNode& Node);

	UFUNCTION(BlueprintNativeEvent, Category = "glTFRuntime", meta = (DisplayName = "On SkeletalMeshComponent Created"))
	void ReceiveOnSkeletalMeshComponentCreated(USkeletalMeshComponent* SkeletalMeshComponent, const FglTFRuntimeNode& Node);

#if 0 // WITH_DIRECTIVE
	UFUNCTION(BlueprintCallable, Category = "glTFRuntime")
	void SetCurveAnimationByName(const FString& CurveAnimationName);
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	bool bAllowNodeAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	bool bStaticMeshesAsSkeletal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	bool bAllowSkeletalAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	bool bAllowPoseAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	bool bAllowCameras;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	bool bAllowLights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	bool bForceSkinnedMeshToRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime")
	int32 RootNodeIndex;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FglTFRuntimeAssetActorNodeProcessed, const FglTFRuntimeNode&, USceneComponent*);
	FglTFRuntimeAssetActorNodeProcessed OnNodeProcessed;

	virtual void PostUnregisterAllComponents() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category="glTFRuntime")
	USceneComponent* AssetRoot;

#if 1 // WITH_DIRECTIVE
	void CustomAddInstanceComponent(UActorComponent* Component);
	AActor* GetComponentOwner();
	class UglTFRuntimeAssetActorComponent* GetAssetComponent();
#endif
};
