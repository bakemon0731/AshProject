// Fill out your copyright notice in the Description page of Project Settings.


#include "AI.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Name.h"


void UAI::SetValueAsGameplayTag(UBlackboardComponent* Target, const FName& KeyName, FGameplayTag TagValue)
{
	if (Target)
	{
		// GameplayTag型のキーに対して値をセットする
		Target->SetValue<UBlackboardKeyType_Name>(KeyName, TagValue.GetTagName());
	}
}
