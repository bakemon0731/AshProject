// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInerface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPlayerInerface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAS_API IPlayerInerface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// 詠唱開始を通知する関数 (引数に詠唱時間を取る)
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category="AbilityState")
	void NotifyStartCast(float ChargeTime);
	
	// 詠唱キャンセルを通知する関数
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category="AbilityState")
	void NotifyCancelCast();
};
