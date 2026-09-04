// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAS_API IDamageable
{
	GENERATED_BODY()
		
	public:
		// -------------------------------------------------------------------
		// インターフェース関数
		// -------------------------------------------------------------------
		virtual int32 GetTeamNumber() const = 0;
};
