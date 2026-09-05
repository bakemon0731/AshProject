// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "GameplayTagContainer.h"
#include "Stats/Stats.h"
#include "BTD_StateGameplayTag.generated.h"

/**
 * 
 */

class UAbilitySystemComponent;

// STATGROUP_AI に "BTD StateGameplayTag Calculate" という統計項目を追加
//DECLARE_CYCLE_STAT(TEXT("BTD StateGameplayTag Calculate"), STAT_BTD_StateGameplayTag_Calculate, STATGROUP_AI);
//実際に何回タグ変化を検知したか
//DECLARE_CYCLE_STAT(TEXT("BTD StateGameplayTag TagChanged"), STAT_BTD_StateGameplayTag_TagChanged, STATGROUP_AI);

// 累積カウンター用（時間ではなく回数を数える）
//DECLARE_DWORD_COUNTER_STAT(TEXT("BTD StateGameplayTag Calculate Count"), STAT_BTD_StateGameplayTag_CalculateCount, STATGROUP_AI);
//DECLARE_DWORD_COUNTER_STAT(TEXT("BTD StateGameplayTag TagChanged Count"), STAT_BTD_StateGameplayTag_TagChangedCount, STATGROUP_AI);

UCLASS()
class GAS_API UBTD_StateGameplayTag : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTD_StateGameplayTag();
	
	//指定するState変数
	UPROPERTY(EditAnywhere,Category="State")
	FGameplayTag StateTag;
	
protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
	
private:
	// Decoratorインスタンスごとにデータを持たせるためのメモリ構造体
	struct FNodeMemory
	{
		TWeakObjectPtr<UBehaviorTreeComponent> CachedBTComp;
		TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
		FDelegateHandle TagDelegateHandle;
	};
	
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FNodeMemory); }
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
	
	void HandleTagChanged(FGameplayTag Tag, int32 NewCount,TWeakObjectPtr<UBehaviorTreeComponent> BTCompWeak);
	
	static UAbilitySystemComponent* GetASCFromOwnerComp(UBehaviorTreeComponent& OwnerComp);
	
	// 呼び出し回数を素朴にカウントする（デバッグ用）
	//mutable int32 CalculateCallCount = 0;
	//int32 TagChangedCallCount = 0;
};
