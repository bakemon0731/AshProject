// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "GAS/GameplayAbilitySystem/Abilities/NexusGameplayAbility.h"
#include "GA_EquipWeaponBase.generated.h"

/**
 * GA_EquipWeaponをC++に移植
 */

class UAC_WeaponComponent;
class AWeaponBase;

UCLASS()
class GAS_API UGA_EquipWeaponBase : public UNexusGameplayAbility
{
	GENERATED_BODY()
	
public:
	// タグと武器クラスの紐づけマップ変数
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Weapon")
	TMap<FGameplayTag,TSubclassOf<AWeaponBase>> WeaponClassMap;
	
protected:
	//EventActivateAbilityFromEventをオーバーライド。
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
