// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusGameplayAbility.h"

UNexusGameplayAbility::UNexusGameplayAbility()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.Active")));//ActivationOwnedTagsにアクティブな共通タグを追加することで、アビリティがアクティブであることを示す。
	
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));//PlayerやキャラクターにState.Deadがついていると他のアビリティはブロック（使用不可）になる。
}

void UNexusGameplayAbility::SetAbilityLevel(int32 NewLevel)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetCurrentAbilitySpec())//もしAbilitySpecが存在する場合、AbilitySpecのレベルをNewLevelに設定する。
	{
		AbilitySpec->Level = NewLevel;
	}
}


bool UNexusGameplayAbility::HasPC() const//HasPC関数ロジック
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
	// アビリティ終了時にイベントを発火
	OnAbilityEnded.Broadcast(this);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
