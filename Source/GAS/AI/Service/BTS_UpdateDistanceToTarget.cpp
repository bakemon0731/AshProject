// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_UpdateDistanceToTarget.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTS_UpdateDistanceToTarget::UBTS_UpdateDistanceToTarget()
{
	NodeName = "UpdateDistanceToTarget_CPP";
	bNotifyTick = true;
}

void UBTS_UpdateDistanceToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!BlackboardComp || !AIC) return ;
	
	//自身のPawnを取得
	APawn* ControlledPawn = AIC->GetPawn();
	if (!ControlledPawn) return ;
	
	//ブラックボードからターゲットActorを取得
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
	if (!TargetActor) return ;
	
	// 距離を計算してブラックボードに書き込む
	float Distance = FVector::Distance(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
	BlackboardComp->SetValueAsFloat(DistanceKey.SelectedKeyName, Distance);
}


