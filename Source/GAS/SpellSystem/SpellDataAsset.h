// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SpellDataAsset.generated.h"

class UGameplayAbility;
class UTexture2D;

UCLASS()
class GAS_API USpellDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 識別用タグ（例: Spell.Fire.FireBall）
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Spell")
	FGameplayTag SpellTag;
	
	//SpellのAbilityClass
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Spell")
	TSubclassOf<UGameplayAbility> AbilityClass;
	
	//SpellのIcon
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Spell")
	TObjectPtr<UTexture2D> Icon;
	
	//Spellの表示名
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Spell")
	FText DisplayName;
	
	//Spellの説明
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Spell",meta = (MultiLine = true))
	FText Description;
	
	//Spellの記憶消費コスト
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category = "Spell")
	int32 Cost = 1;
};
