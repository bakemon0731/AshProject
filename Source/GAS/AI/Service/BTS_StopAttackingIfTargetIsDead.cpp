// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_StopAttackingIfTargetIsDead.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/AI/AIController/EnemyAIController.h"


UBTS_StopAttackingIfTargetIsDead::UBTS_StopAttackingIfTargetIsDead()
{
	// ビヘイビアツリー上で表示されるノード名
	NodeName = "StopAttackingIfTargetIsDead_CPP";
	
	// Tickを実行するために必要
	bNotifyTick = true;
}

void UBTS_StopAttackingIfTargetIsDead::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	//Blackboardの参照を取得
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;
	
	//BlackboardのキーからActor(AttackTarget)を取得
	// GetSelectedBlackboardKey() は、エディタ上で指定したキー(Blackboard Key)を指す。
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(GetSelectedBlackboardKey()));
	if (!TargetActor) return;
	
	//ActorからAbility System Componentを取得
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (ASC)
	{
		//指定したGameplayTag（State.Dead）を持っているかチェック
		if (ASC->HasMatchingGameplayTag(TagToCheck))
		{
			//BT所有者のAIControllerを取得してキャスト
			AEnemyAIController* EnemyAIC = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
			if (EnemyAIC)
			{
				//状態をPassiveにする関数を呼び出し
				EnemyAIC->SetStateAsPassive();
			}
		}
	}
}
