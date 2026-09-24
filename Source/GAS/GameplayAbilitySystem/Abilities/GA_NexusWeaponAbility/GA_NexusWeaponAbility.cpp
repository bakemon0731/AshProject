// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_NexusWeaponAbility.h"
#include "GAS/Weapon/WeaponBase/WeaponBase.h"
#include "GAS/Weapon/WeaponComponent/AC_WeaponComponent.h"

bool UGA_NexusWeaponAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	// クールダウンやコスト等を通過しているか確認
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	//Break GameplayAbilityActorInfo -> AvatarActor の取得
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	
	//Get Component by Class (WeaponManagerComponentを取得)
	UAC_WeaponComponent* WeaponManager = AvatarActor->FindComponentByClass<UAC_WeaponComponent>();
	
	//Is Valid 判定（取得できなければ発動不可）
	if (!WeaponManager)
	{
		return false;
	}
	
	//装備中の武器 (EquippedWeapon) を取得
	AWeaponBase* CurrentWeapon = WeaponManager->EquippedWeapon;
	if (!CurrentWeapon)
	{
		return false;
	}
	
	//Get Class して RequiresEquippedWeapon と一致するか判定
	if (RequiresEquippedWeapon)
	{
		return CurrentWeapon->GetClass() == RequiresEquippedWeapon;
	}
	return true;
	
}
