// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GameplayAbilitySystem/Abilities/GA_NexusWeaponAbility/GA_NexusWeaponAbility.h"
#include "GA_UnequipWeaponBase.generated.h"

/**
 * 
 */
UCLASS()
class GAS_API UGA_UnequipWeaponBase : public UGA_NexusWeaponAbility
{
	GENERATED_BODY()
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
