// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayEffectTypes.h"
#include "EnemyAIController.generated.h"

UCLASS()
class GAS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyAIController();
	
	// ステートをPassiveに変更する関数
	UFUNCTION(BlueprintCallable,Category="State")
	void SetStateAsPassive();
	
protected:
	// BPの Event On Possess に相当する関数
	virtual void OnPossess(APawn* InPawn) override;
	
	// タイマーで周期実行される関数
	void CheckIfForgottenSeeActor();
	
	// Set Value as FloatノードのKeyNameの変数
	UPROPERTY(EditDefaultsOnly,Category="Blackboard")
	FName AttackRediusKeyName = "AttackRadius";
	
	// Set Value as FloatノードのKeyNameの変数
	UPROPERTY(EditDefaultsOnly,Category="Blackboard")
	FName DefendRadiusKeyName = "DefendRadius";
	
	// 付与する GameplayEffect (GE_Passive)
	UPROPERTY(EditDefaultsOnly,Category="StateEffect")
	TSubclassOf<class UGameplayEffect>PassiveStateEffect;
	
	// タイマー制御用のハンドル (BPの Check Forgotten Actor Timer 変数)
	FTimerHandle CheckForgottenActorTimer;
	
	// 現在のStateエフェクトのハンドル (BPの Current State Effect 変数)
	FActiveGameplayEffectHandle CurrentStateEffect;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
