// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusGameplayAbility.h"

UNexusGameplayAbility::UNexusGameplayAbility()
{
	//ActivationOwnedTagsにアクティブな共通タグを追加することで、アビリティがアクティブであることを示す。
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.Active")));
	
	//PlayerやキャラクターにState.Deadがついていると他のアビリティはブロック（使用不可）になる。
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
}

void UNexusGameplayAbility::SetAbilityLevel(int32 NewLevel)
{
	//もしAbilitySpecが存在する場合、AbilitySpecのレベルをNewLevelに設定する。
	if (FGameplayAbilitySpec* AbilitySpec = GetCurrentAbilitySpec())
	{
		AbilitySpec->Level = NewLevel;
	}
}

//HasPC関数ロジック
bool UNexusGameplayAbility::HasPC() const
{
	const APawn* PawnObject = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!PawnObject)
	{
		return false;
	}
	return PawnObject->GetController()->IsA<APlayerController>();
}

void UNexusGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateEndAbility, 
	bool bWasCancelled)
{
	// アビリティ終了時にイベントを発火「呼ぶ（Call）」
	OnAbilityEnded.Broadcast(this);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
