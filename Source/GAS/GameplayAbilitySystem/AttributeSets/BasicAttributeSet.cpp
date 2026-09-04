// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Pawn.h"
#include "Perception/AISense_Damage.h"
#include "Net/UnrealNetwork.h"

UBasicAttributeSet::UBasicAttributeSet()
{
	Health = 100.f;
	MaxHealth = 100.f;
	Stamina = 100.f;
	MaxStamina = 100.f;
	Damage = 0.f;
	Shield = 0.f;
	MaxShield = 100.f;
}

void UBasicAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);//親関数

	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Health, COND_None, REPNOTIFY_Always);
	//UBasicAttributeSet - このクラスの
	//Health - Health プロパティを
	//COND_None - 条件なく（常に）
	//REPNOTIFY_Always - 常に通知する
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
}//↑ 「この4つの属性をネットワーク同期の対象にします」と宣言している関数。

void UBasicAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);//親関数

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}	
}// ↑ 属性値が最小値（0）と最大値（MaxHealth または MaxStamina）の範囲を超えないようにチェックして、自動的に修正する


void UBasicAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);//親関数
	
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())//もし、GameplayEffect によって Damage 属性が変更された場合
	{
		float TotalDamage = GetDamage();// Damage 属性の値を取得する
		SetDamage(0.f);// Damage 属性を0にリセットする
		
		
		//ここからAIへの感知（ダメージイベント）送信を追加
		if (TotalDamage > 0.f)//Damageをうけた場合
		{
			//ターゲット（ダメージを受けた側＝AI自身）を取得
			AActor* TargetActor = GetOwningActor();
			
			// ソース（ダメージを与えた側＝プレイヤーや発射物のInstigator）を取得
			AActor* SourceActor = Data.EffectSpec.GetContext().GetInstigator();
			
			// SourceActorがAPawn（キャラクターやプレイヤー）にキャストできるか確認する
			// ※燃える床などの環境ダメージは通常Pawnではないため、ここで弾く
			APawn* Sourcepawn = Cast<APawn>(SourceActor);
			
			// ターゲットが存在し、かつ攻撃者がPawnである場合のみ処理を進める
			if (TargetActor && Sourcepawn)
			{
				// 念のため、自分自身の攻撃（自傷ダメージなど）ではないことも確認
				if (TargetActor != Sourcepawn)
				{
					UAISense_Damage::ReportDamageEvent(
						TargetActor,//WorldContext
						TargetActor,//ダメージを受けたActor
						Sourcepawn,// 攻撃者（APawn）
						TotalDamage,// ダメージ量
						TargetActor->GetActorLocation(),// イベント発生位置
						TargetActor->GetActorLocation(),// ヒット位置
						FName("Damage")// イベントタグ
						);
				}
			}
		}
		
		
		float CurrentShield = GetShield();//現在のShieldを取得する
		if (CurrentShield > 0.f)//もしシールドを装備している場合
		{
			SetShield(CurrentShield - TotalDamage);//現在のシールド値からダメージを差し引いた値をセットする。
			float RemaningDamage = TotalDamage - CurrentShield; //残りのダメージを計算する
			
			if (RemaningDamage > 0.f)//もし残りのダメージが0より大きい場合 = シールド値をダメージが上回った場合
			{
				SetHealth(GetHealth() - RemaningDamage); // Health 属性から残りのダメージを引く
			}
		}
		else// シールドをそうびしていない場合
		{
			SetHealth(GetHealth() - TotalDamage);// Health 属性から Damage 属性の値を引く
		}
		
		if (Data.EffectSpec.Def->GetAssetTags().HasTag(FGameplayTag::RequestGameplayTag("Effects.HitReaction"))// もし、GameplayEffect に "Effects.HitReaction" タグが付いていたら
			&& Data.EvaluatedData.Magnitude != 0.f)// かつ、GameplayEffect の効果量が0でない場合
		{
			FGameplayTagContainer HitReactionTagContainer;
			HitReactionTagContainer.AddTag(FGameplayTag::RequestGameplayTag("GameplayAbility.HitReaction"));// FGameplayTagContanerを作成して、"GameplayAbility.HitReaction"タグを追加する。
			GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(HitReactionTagContainer);// 所有しているAbilitySystemComponentに対して、"GameplayAbility.HitReaction"タグに紐づいたアビリティを起動する処理
		}
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(GetHealth());// Health が変更された場合の処理
		
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(GetStamina());// Stamina が変更された場合の処理
	}
}


void UBasicAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetHealthAttribute() && NewValue <= 0.f)//もし、Health属性が0以下になった場合
	{
		FGameplayTagContainer DeathAbilityTagContainer;
		DeathAbilityTagContainer.AddTag(FGameplayTag::RequestGameplayTag("GameplayAbility.Death"));// FGameplayTagContanerを作成して、"GameplayAbility.Death"タグを追加する。
		GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(DeathAbilityTagContainer);//所有しているAbilitySystemComponentに対して、"GameplayAbility.Death"タグに紐づいたアビリティを起動する処理
	}
	
	if (Attribute == GetShieldAttribute())//もし、Shield属性が変更された場合
	{
		if (NewValue > 0.f && OldValue <= 0.f)//もし、新しいシールド値が0より大きいかつ古いシールド値が0以下の場合は、GameplayCueを発動する。
		{
			GetOwningAbilitySystemComponent()->AddGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.ShieldUp"));//所有しているAbilitySystemComponentに対して、"GameplayCue.ShieldUp"タグに紐づいたゲームプレイキューを発動する処理
		}	
		else if(NewValue <= 0.f && OldValue > 0.f)//もし、新しいシールド値が0以下かつ古いシールド値が0より大きい場合は、"GameplayCue.ShieldUp"タグのGameplayCueを解除し、"GameplayCue.ShieldDown"タグのGameplayCueを発動する。
		{
			GetOwningAbilitySystemComponent()->RemoveGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.ShieldUp"));//所有しているAbilitySystemComponentに対して、"GameplayCue.ShieldUp"タグに紐づいたゲームプレイキューを解除する処理
			GetOwningAbilitySystemComponent()->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.ShieldDown"));//所有しているAbilitySystemComponentに対して、"GameplayCue.ShieldDown"タグに紐づいたゲームプレイキューを実行する処理
		}	
	}
	
}
