// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_SpellFireBase.h"

#include "AbilitySystemComponent.h"
#include "GAS/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"

UGA_SpellFireBase::UGA_SpellFireBase()
{
	// デフォルトで用意したGameplayTagをセット
	HPCostTag = FGameplayTag::RequestGameplayTag(FName("Data.Cost.HP"));
}

bool UGA_SpellFireBase::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}
	
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return false;
	}
	
	// 現在のHPを取得し、コストと比較
	float CurrentHealth = ASC->GetNumericAttribute(UBasicAttributeSet::GetHealthAttribute());
	float CostValue = HPCost.GetValueAtLevel(GetAbilityLevel());
	
	//HPがコスト未満なら発動不可
	return CurrentHealth >= CostValue;
}

void UGA_SpellFireBase::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!HPCostGEClass)
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}
	
	// GE Specを作成
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());
	
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HPCostGEClass,GetAbilityLevel(),Context);
	if (SpecHandle.IsValid())
	{
		// SetByCallerで自傷ダメージ（マイナス値）を設定
		float CostValue = HPCost.GetValueAtLevel(GetAbilityLevel());
		SpecHandle.Data->SetSetByCallerMagnitude(HPCostTag,-CostValue);
		
		// 自分自身に適用
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

