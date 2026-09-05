// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "GameplayTagContainer.h"
#include "BTS_StopAttackingIfTargetIsDead.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UBTS_StopAttackingIfTargetIsDead : public UBTService_BlackboardBase
{
	GENERATED_BODY()
	
public:
		UBTS_StopAttackingIfTargetIsDead();
	
protected:
	// BPの「Event Receive Tick AI」に相当する関数
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	// エディタ（詳細パネル）から設定できるようにするタグ（デフォルトは空）
	UPROPERTY(EditAnywhere,Category="GAS")
	FGameplayTag TagToCheck;
};
