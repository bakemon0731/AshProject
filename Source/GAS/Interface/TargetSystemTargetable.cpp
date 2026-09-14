// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetSystemTargetable.h"


// デフォルトでは常にターゲット可能（true）とする
bool ITargetSystemTargetable::IsTargetable_Implementation() const
{
	return true;
}