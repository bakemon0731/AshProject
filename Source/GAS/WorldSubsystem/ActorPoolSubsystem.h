// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ActorPoolSubsystem.generated.h"

/**
 * クラス単位で使い回すActorを保持する汎用オブジェクトプール。
 * 発射物専用にせず、他のワンショットActor(ヒットエフェクト等)にも流用できる設計。
 */

// TMapの値としてTArrayを直接持てないため、USTRUCTでラップする
USTRUCT()
struct FPooledActorArray
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> Actors;
};

UCLASS()
class GAS_API UActorPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	// プールからアクタを1つ取り出す。プールが空なら新規SpawnActorする関数。
	UFUNCTION(BlueprintCallable,Category = "Pooling",meta = (DeterminesOutputType = "ActorClass"))
	AActor* AcquireActor(TSubclassOf<AActor> ActorClass,
		const FTransform& SpawnTransform,
		AActor*Owner = nullptr,
		APawn* SpawnInstigator = nullptr
		);
	
	// Actorを非表示・コリジョン無効化してプールに返却する(Destroyしない)関数。
	UFUNCTION(BlueprintCallable,Category = "Pooling")
	void ReleaseActor(AActor* Actor);
	
	//ロード時のスパイクを避けるため、事前にN体生成してプールへ入れておく
	UFUNCTION(BlueprintCallable,Category = "Pooling")
	void PrewarmPool(TSubclassOf<AActor> ActorClass, int32 Count);
	
private:
	UPROPERTY()
	TMap<TSubclassOf<AActor>,FPooledActorArray> Pools;
};
