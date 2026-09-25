// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_UnequipWeaponBase.h"
#include "GAS/Weapon/WeaponComponent/AC_WeaponComponent.h"

void UGA_UnequipWeaponBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// コストやクールダウンの適用チェック
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true,true);
		return;
	}
	
	//アバターアクターの確認
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		//ローカル変数にアバターアクターを保存
		AActor* AvatarActor = ActorInfo->AvatarActor.Get();
		
		//武器コンポーネントを取得。
		if (UAC_WeaponComponent* WeaponManager = AvatarActor->FindComponentByClass<UAC_WeaponComponent>())
		{
			//武器解除を実行
			WeaponManager->UnequipWeapon();
		}
	}
	// アビリティを終了
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
