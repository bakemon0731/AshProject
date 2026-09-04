// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NexusGameplayAbility.generated.h"

/**
 * 
 */

UENUM(BlueprintType)// ブループリントで使用可能にするためのマクロ
enum class EAbilityInputID : uint8//入力列挙型の定義
{
	None UMETA(DisplayName = "None"),// 0
	PrimaryAbility UMETA(DisplayName = "Primary Ability"),// 1
	SecondaryAbility UMETA(DisplayName = "Secondary Ability"),// 2
	DefensiveAbility UMETA(DisplayName = "Defensive Ability"),// 3
	MovementAbility UMETA(DisplayName = "Movement Ability"),// 4
};

UCLASS()
class GAS_API UNexusGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UNexusGameplayAbility();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")//UIに表示するかどうかの変数
	bool ShouldShowInAbilitiesBar = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoGrantAbility")//自動的にAbilityを発動するかどうかの変数
	bool AutoActivateWhenGranted = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")//入力ID変数
	EAbilityInputID AbilityInputID = EAbilityInputID::None;
	
	UFUNCTION(BlueprintCallable, Category = "Ability")//すでに適用されたGameplayAbilityのレベルを更新
	void SetAbilityLevel(int32 NewLevel);
    
private:	

	UFUNCTION(BlueprintCallable, Category = "Helpers")// プレイヤーコントローラーを持っているかいないかを返す関数。
	bool HasPC() const;
};
