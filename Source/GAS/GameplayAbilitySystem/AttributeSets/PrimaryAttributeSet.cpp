// Fill out your copyright notice in the Description page of Project Settings.


#include "PrimaryAttributeSet.h"
#include "Net/UnrealNetwork.h"

UPrimaryAttributeSet::UPrimaryAttributeSet()
{
	Knowledge = 0.f;
	Willpower = 0.f;
	Agility   = 0.f;
	Vitality  = 0.f;
	MaxMemoryCapacity = 0.f;
}

//変数の同期
void UPrimaryAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UPrimaryAttributeSet,Knowledge,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPrimaryAttributeSet,Willpower,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPrimaryAttributeSet,Agility,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPrimaryAttributeSet,Vitality,COND_None,REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UPrimaryAttributeSet,MaxMemoryCapacity,COND_None,REPNOTIFY_Always);
}


