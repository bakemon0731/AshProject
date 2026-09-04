// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "DamageCalculationModifier.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UDamageCalculationModifier : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	UDamageCalculationModifier();
	
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
	
private:
	// 属性のキャプチャ定義
	FGameplayEffectAttributeCaptureDefinition ArmorDef;
	FGameplayEffectAttributeCaptureDefinition StrengthDef;
	
};
