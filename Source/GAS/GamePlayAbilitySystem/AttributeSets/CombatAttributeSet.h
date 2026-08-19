// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "CombatAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UCombatAttributeSet();
	
	//Armor Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Armor",ReplicatedUsing=OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS_BASIC(UCombatAttributeSet, Armor);//ATTRIBUTE_ACCESSORS_BASIC();<<<各属性に対して、手動でゲッター・セッター関数を書かなくても自動で生成してくれる便利なマクロ
	
	
	UPROPERTY(BlueprintReadOnly, Category = "MaxArmor",ReplicatedUsing=OnRep_MaxArmor)
	FGameplayAttributeData MaxArmor;
	ATTRIBUTE_ACCESSORS_BASIC(UCombatAttributeSet, MaxArmor);

	//Strength Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Strength",ReplicatedUsing=OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS_BASIC(UCombatAttributeSet, Strength);

	UPROPERTY(BlueprintReadOnly, Category = "MaxStrength",ReplicatedUsing=OnRep_MaxStrength)
	FGameplayAttributeData MaxStrength;
	ATTRIBUTE_ACCESSORS_BASIC(UCombatAttributeSet, MaxStrength);
	
	
protected:
	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Armor, OldValue);
		// ↑ Armor が変更されたことをゲーム全体に通知
	}
	
	UFUNCTION()
	void OnRep_MaxArmor(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, MaxArmor, OldValue);
		// ↑ MaxArmor が変更されたことをゲーム全体に通知
	}

	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Strength, OldValue);
		// ↑ Strength が変更されたことをゲーム全体に通知
	}

	UFUNCTION()
	void OnRep_MaxStrength(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, MaxStrength, OldValue);
		// ↑ MaxStrength が変更されたことをゲーム全体に通知
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	// ↑ 「このクラスの 属性をネットワーク同期してね」とエンジンに指示する関数
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	//↑ 属性が変更される前に呼ばれる関数。属性の値を制限するために使用される。
	
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	// ↑ ゲームプレイエフェクトが実行された後に呼ばれる関数。属性の値を制限するために使用される。
};
