// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "NexusCharacterBase.generated.h"

UCLASS()
class GAS_API ANexusCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	
	ANexusCharacterBase();//キャラクターの初期化
	
	// AbilitySystemComponent<<<ゲーム中にアビリティを使うためのコンポーネント
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")// キャラクターに付与される基本属性セットを管理するための変数
	class UBasicAttributeSet* BasicAttributeSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")// キャラクターに付与される戦闘用の属性セット（Armor、Strengthなど）を管理するための変数
	class UCombatAttributeSet* CombatAttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TeamNumber")
	int32 TeamNumber = 2;
	
	
protected://マルチプレイヤーでのデータ同期に必要
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;
	//変数：AscReplicationMode
	//型：EGameplayEffectReplicationMode（ゲームモード）
	//デフォルト値：Mixed（混合モード）
	//機能：ネットワークでアビリティの情報をどう同期するか
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability System")// キャラクターがゲーム開始時に持つべきアビリティのリストを保存する変数
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;
	
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_PlayerState() override;
	
	virtual void OnDeathTagChanged(const FGameplayTag CallbackTag, int32 NewCount);// 死亡タグが変化した時に呼ばれる関数（デフォルトでは何もしない）
	
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")// 死亡時の処理を実装する関数
	void HandleDeath();
	
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")// アビリティを付与する関数
	TArray<FGameplayAbilitySpecHandle>GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant,int32 Level = 1);
	
	
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")// アビリティを削除する関数
	void RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove);

	
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")// アビリティの変更を知らせるゲームプレイイベントを送る関数
	void SendAbilitiesChangedEvent();
	
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "AbilitySystem")// サーバーRPC呼び出し関数　＝　クライアントからサーバーにイベントを送信する関数
	void ServerSendGameplayEventToSelf(FGameplayEventData EventData );
	
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "AbilitySystem")// マルチキャストRPC呼び出し関数　＝　サーバーから全クライアントにイベントを送信する関数
	void MultiSendGameplayEventToSelf(AActor*TargetActor, FGameplayEventData EventData);
	
};
