// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageCalculationModifier.h"
#include "GAS/GamePlayAbilitySystem/AttributeSets/CombatAttributeSet.h"
#include "GAS/GameplayAbilitySystem/AttributeSets/PrimaryAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagsManager.h"


UDamageCalculationModifier::UDamageCalculationModifier()
{
	// TargetのArmorをキャプチャ
	ArmorDef = FGameplayEffectAttributeCaptureDefinition(UCombatAttributeSet::GetArmorAttribute(),EGameplayEffectAttributeCaptureSource::Target,false);
	RelevantAttributesToCapture.Add(ArmorDef);
	
	// SourceのStrengthをキャプチャ
	StrengthDef = FGameplayEffectAttributeCaptureDefinition(UCombatAttributeSet::GetStrengthAttribute(),EGameplayEffectAttributeCaptureSource::Source,false);
	RelevantAttributesToCapture.Add(StrengthDef);
	
	//SourceのWillpowerをキャプチャ（魔法ダメージ用）
	WillPowerDef = FGameplayEffectAttributeCaptureDefinition(UCombatAttributeSet::GetStrengthAttribute(),EGameplayEffectAttributeCaptureSource::Source,false);
	RelevantAttributesToCapture.Add(WillPowerDef);
}

float UDamageCalculationModifier::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	// 各Attributeの取得
	float TargetArmor = 0.f;
	GetCapturedAttributeMagnitude(ArmorDef,Spec,EvaluateParameters,TargetArmor);
	float SourceStrength = 0.f;
	GetCapturedAttributeMagnitude(StrengthDef,Spec,EvaluateParameters,SourceStrength);
	float SourceWillPower = 0.f;
	GetCapturedAttributeMagnitude(WillPowerDef,Spec,EvaluateParameters,SourceStrength);
	
	// SetByCallerからBaseDamageを取得
	float BaseDamage = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")),false,0.0f);
	
	// ダメージ識別判定（GEのアセットタグを見る）
	const bool bIsPhysical = Spec.Def->GetAssetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("Data.DamageType.Physical")));
	const bool bIsMagic = Spec.Def->GetAssetTags().HasTag(FGameplayTag::RequestGameplayTag(FName("Data.DamageType.Magic")));
	
	float OffensiveMultiplier = 1.0f;
	if (bIsPhysical)
	{
		OffensiveMultiplier = 1.0f + (0.05f * SourceStrength);
	}
	else if (bIsMagic)
	{
		OffensiveMultiplier = 1.0f + (0.03f * SourceWillPower);
	}
	
	// ダメージ計算
	float TotalDamage = (BaseDamage * OffensiveMultiplier) / (1.0f + (0.05f * TargetArmor));
	
	
	// シールドタグによる判定
	FGameplayTag ShieldTag = FGameplayTag::RequestGameplayTag(FName("Status.Buff.Shield"));
	if (TargetTags && TargetTags->HasTagExact(ShieldTag))
	{
		// シールドがあればダメージ0を返す
		return 0.0f;
	}
	
	// なければ計算したダメージを返す
	return TotalDamage;
	
}


