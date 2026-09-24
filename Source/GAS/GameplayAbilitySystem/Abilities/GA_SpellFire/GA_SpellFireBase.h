// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GameplayAbilitySystem/Abilities/GA_NexusWeaponAbility/GA_NexusWeaponAbility.h"
#include "GA_SpellFireBase.generated.h"

class UAnimMontage;
class UGameplayEffect;

UCLASS()
class GAS_API UGA_SpellFireBase : public UGA_NexusWeaponAbility
{
	GENERATED_BODY()

public:
	
	UGA_SpellFireBase();
	
	// 発動可能かどうかのコストチェック
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	
	// コストの適用処理
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;
	
protected:
	
	// 呪文ごとのHP消費量
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|Cost")
	FScalableFloat HPCost;
	
	// 共通のHP消費GEクラス
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|Cost")
	TSubclassOf<UGameplayEffect> HPCostGEClass;
	
	// SetByCaller用のタグ（Data.Cost.HP）
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|Cost")
	FGameplayTag HPCostTag;
	
	//詠唱中のGEデバフ。
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|Config")
	TSubclassOf<UGameplayEffect> DebuffGEClass;
	
	//発射に必要な詠唱時間。
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|Config")
	float RequiredChargeTime; 
	
	//詠唱中のGCタグ。
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|Config")
	FGameplayTag SpellChantGCTag;
	
	//共通のダメージGEクラス
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|DamageConfig")
	TSubclassOf<UGameplayEffect> DamageGEClass;
	
	//ダメージ量のマグニチュード変数。
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|DamageConfig")
	float DamageMagnitude;
	
	//発射物のSpeed。
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|ProjectileConfig")
	float ProjectileSpeed = 2000.0f;
	
	//Traceの適用EffectClass。
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Spell|TraceConfig")
	TSubclassOf<UGameplayEffect> TraceEffect;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Spell|ParentConfig")
	UAnimMontage* ChargeMontage;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Spell|ParentConfig")
	UAnimMontage* ChargeMontage_Loop;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Spell|ParentConfig")
	UAnimMontage* FireMontage;
	
	//DebuffGEClass変数のハンドル変数
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category= "Spell|ParentConfig")
	FActiveGameplayEffectHandle DebuffGEHandle;
	
	//詠唱完了しボタンをReleaseしたかどうか。
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category= "Spell|ParentConfig")
	bool IsChargeFire;
	
	//最大時間詠唱が完了したかどうか。
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category= "Spell|ParentConfig")
	bool IsChargeComplete;
};
