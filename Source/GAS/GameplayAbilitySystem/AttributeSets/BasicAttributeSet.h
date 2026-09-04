// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BasicAttributeSet.generated.h"

/**
 * 
 */

UCLASS()
class GAS_API UBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:	 // <<<ここ以降のメンバーは外部からアクセス可能
	
	UBasicAttributeSet();
	
	//Health Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Health",ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Health);
	//ATTRIBUTE_ACCESSORS_BASIC();<<<各属性に対して、手動でゲッター・セッター関数を書かなくても自動で生成してくれる便利なマクロ
	
	UPROPERTY(BlueprintReadOnly, Category = "MaxHealth",ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxHealth);

	//Stamina Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Stamina",ReplicatedUsing=OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Stamina);

	UPROPERTY(BlueprintReadOnly, Category = "MaxStamina",ReplicatedUsing=OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxStamina);
	
	//ダメージ属性（複製されない）
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Damage);
	
	//Shield Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Shield",ReplicatedUsing=OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Shield);

	UPROPERTY(BlueprintReadOnly, Category = "MaxShield",ReplicatedUsing=OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxShield);
	
public:
	// ネットワークで同期時に呼ばれる
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Health, OldValue);
		// ↑ Health が変更されたことをゲーム全体に通知
	}
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxHealth, OldValue);
		// ↑ MaxHealth が変更されたことをゲーム全体に通知
	}

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Stamina, OldValue);
		// ↑ Stamina が変更されたことをゲーム全体に通知
	}

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxStamina, OldValue);
		// ↑ MaxStamina が変更されたことをゲーム全体に通知
	}
	
	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Shield, OldValue);
		// ↑ Shield が変更されたことをゲーム全体に通知
	}
	
	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxShield, OldValue);
		// ↑ MaxShield が変更されたことをゲーム全体に通知
	}
	
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// ↑ 「このクラスの Health、MaxHealth、Stamina、MaxStamina をネットワーク同期してね」とエンジンに指示する関数
	
	virtual auto PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) -> void override;
	//↑ 属性が変更される前に呼ばれる関数。属性の値を制限するために使用される。
	
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	// ↑ ゲームプレイエフェクトが実行された後に呼ばれる関数。属性の値を制限するために使用される。
	
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
};
