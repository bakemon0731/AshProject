// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableActor.generated.h"

/**
* ActorPoolSubsystem によって使い回される Actor が実装するインターフェース。
 * Destroy/Spawn の代わりに「アクティブ化」「非アクティブ化」のタイミングで
 * 状態のリセットや演出の後始末を行うためのフックを提供する。
 */

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UPoolableActor : public UInterface
{
	GENERATED_BODY()
};

class GAS_API IPoolableActor
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// プールから取り出され、ワールドで再度有効になった直後に呼ばれる（Poolが有効を通知）
	UFUNCTION(BlueprintNativeEvent,Category = "Pooling")
	void OnActivatedFromPool();
	
	// プールに返却され、非表示・コリジョン無効化された直後に呼ばれる（Poolが無効を通知）
	UFUNCTION(BlueprintNativeEvent,Category = "Pooling")
	void OnReturnToPool();
	
};
