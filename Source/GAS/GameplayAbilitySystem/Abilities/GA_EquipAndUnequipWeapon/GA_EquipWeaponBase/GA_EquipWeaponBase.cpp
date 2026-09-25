// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_EquipWeaponBase.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/Weapon/WeaponComponent/AC_WeaponComponent.h"
#include "GAS/Weapon/WeaponBase/WeaponBase.h"

void UGA_EquipWeaponBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// コストやクールダウンの適用チェック
	if (!CommitAbility(Handle,ActorInfo,ActivationInfo))
	{
        // コンポーネントが存在するか確認 
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
		return;
	}
	
	// アバターアクターとイベントデータの存在確認
	if (ActorInfo && ActorInfo->AvatarActor.IsValid() && TriggerEventData)
	{
		AActor* AvatarActor = ActorInfo->AvatarActor.Get();
		
		//アクターからUAC_WeaponComponentを取得。
		UAC_WeaponComponent* WeaponManager = AvatarActor->FindComponentByClass<UAC_WeaponComponent>();
		
		// コンポーネントが存在するか確認
		if (WeaponManager)
		{
			// トリガーされたイベントデータの TargetTags から武器タグを取得する
			FGameplayTag WeaponTag;
			if (TriggerEventData && !TriggerEventData->TargetTags.IsEmpty())
			{
				// Payloadの Target Tags に設定された最初のタグ(index0)を取得
				WeaponTag = TriggerEventData->TargetTags.GetByIndex(0);
			}
			
			// Map変数から対応する武器クラスを検索
			if (const TSubclassOf<AWeaponBase>* FoundWeaponClassPtr = WeaponClassMap.Find(WeaponTag))
			{
				// キーが存在し、かつ登録されている武器クラスが valid (Noneではない) かチェック
				if (FoundWeaponClassPtr && *FoundWeaponClassPtr)
				{
					TSubclassOf<AWeaponBase> WeaponClassToEquip = *FoundWeaponClassPtr;
					
					// 装備処理を実行
					WeaponManager->EquipWeapon(WeaponClassToEquip);
				}
			}
		}
	}
	// アビリティを終了
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_EquipWeaponBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
