// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetSystemTargetable.generated.h"


UINTERFACE()
class UTargetSystemTargetable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAS_API ITargetSystemTargetable
{
	GENERATED_BODY()

public:
	// ターゲット可能かどうかを判定する関数（C++でもBPでも上書き可能）
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent,Category = "TargetSystem")
	bool IsTargetable() const;
	
	virtual bool IsTargetable_Implementation() const;
};
