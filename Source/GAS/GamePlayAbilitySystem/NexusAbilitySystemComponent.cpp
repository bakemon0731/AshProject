// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusAbilitySystemComponent.h"

#include "Characters/NexusCharacterBase.h"


// Sets default values for this component's properties
UNexusAbilitySystemComponent::UNexusAbilitySystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UNexusAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// サーバーとクライアント間でアビリティ変更を同期させるための関数
void UNexusAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();// 親クラス(UAbilitySystemComponent)の同期処理を実行
	
	ANexusCharacterBase* Character = Cast<ANexusCharacterBase>(GetOwner());// このコンポーネントの所有者(キャラクター)を取得
	if (!Character) return;// キャストに失敗したら終了（安全チェック）
	
	
bool bAbilitiesChanged = false;
	if (LastActivatableAbilities.Num() != ActivatableAbilities.Items.Num())// 前回のアクティブなアビリティと現在のアクティブなアビリティの数が異なる場合
	{
		bAbilitiesChanged = true;
	}
	else
	{
		for (int i = 0; i < ActivatableAbilities.Items.Num(); i++)
		{
			if (LastActivatableAbilities[i].Ability != ActivatableAbilities.Items[i].Ability)// 前回のアクティブなアビリティと現在のアクティブなアビリティが異なる場合
			{
				bAbilitiesChanged = true;
				break;
			}
		}	
	}
	if (bAbilitiesChanged)
	{
		Character->SendAbilitiesChangedEvent();// キャラクターのイベント発火関数を呼ぶ
		LastActivatableAbilities = ActivatableAbilities.Items;// 現在のアクティブなアビリティを保存
	}
}


// Called every frame
void UNexusAbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

