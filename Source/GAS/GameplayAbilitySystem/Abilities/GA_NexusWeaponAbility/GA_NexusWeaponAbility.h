// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/GameplayAbilitySystem/Abilities/NexusGameplayAbility.h"
#include "GA_NexusWeaponAbility.generated.h"

class AWeaponBase;
class UAC_WeaponComponent;


UCLASS()
class GAS_API UGA_NexusWeaponAbility : public UNexusGameplayAbility
{
	GENERATED_BODY()

public:

	// CanActivateAbility のオーバーライド
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr
	)const override;
	
protected:
	// このGAを発動する「必要な装備武器」の変数。
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= "Weapon")
	TSubclassOf<AWeaponBase> RequiresEquippedWeapon;
};