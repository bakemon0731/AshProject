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
	
};
