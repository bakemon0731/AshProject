// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Attack.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UBTT_Attack : public UBTTaskNode
{
	GENERATED_BODY()
	
	UBTT_Attack();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	//実行するGameplayAbility変数
	UPROPERTY(EditAnywhere,Category= "AI")
	TSubclassOf<class UGameplayAbility> AbilityToActivate;
	
	UFUNCTION()
	void OnMyAbilityEnded(UGameplayAbility* Ability);
	
private:
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
