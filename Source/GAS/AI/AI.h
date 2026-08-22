// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UAI : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	public:
	// Blackboardに直接Gameplay Tagをセットする関数
	UFUNCTION(BlueprintCallable,Category="Blackboard|GameplayTags")
	static void SetValueAsGameplayTag(UBlackboardComponent* Target, 
		const FName& KeyName, 
		FGameplayTag TagValue
		);
};
