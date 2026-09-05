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


//--------------------------------------------------
//イベントディスパッチャーの定義
// --------------------------------------------------
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEndedSignature, UGameplayAbility*, Ability);

UCLASS()
class GAS_API UNexusGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	
	UNexusGameplayAbility();
	
	//UIに表示するかどうかの変数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool ShouldShowInAbilitiesBar = false;
	
	//自動的にAbilityを発動するかどうかの変数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoGrantAbility")
	bool AutoActivateWhenGranted = false;
	
	//入力ID変数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EAbilityInputID AbilityInputID = EAbilityInputID::None;

	
	//すでに適用されたGameplayAbilityのレベルを更新
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void SetAbilityLevel(int32 NewLevel);
	
	//EndAbilityしたことをイベントディスパッチする
	UPROPERTY(BlueprintAssignable, Category = "Helpers")
	FOnAbilityEndedSignature OnAbilityEnded;
    
private:	

	// プレイヤーコントローラーを持っているかいないかを返す関数。
	UFUNCTION(BlueprintCallable, Category = "AbilityEnd")
	bool HasPC() const;
	
	
	void EndAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled);
};
