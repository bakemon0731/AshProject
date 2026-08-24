// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayEffectTypes.h"
#include "Perception/AIPerceptionTypes.h"//AIの感知データ（FAIStimulus）を扱うために必要
#include "EnemyAIController.generated.h"

//前方宣言
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

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
	// AI Perception コンポーネント本体
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AIPerception")
	UAIPerceptionComponent* PerceptionComp;
	
	// 各感覚のコンフィグ（BPの詳細パネルで数値を調整可能）
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AIPerception")
	UAISenseConfig_Sight* SightConfig;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AIPerception")
	UAISenseConfig_Hearing* HearingConfig;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="AIPerception")
	UAISenseConfig_Damage* DamageConfig;
	
	// BPの Event On Possess に相当する関数
	virtual void OnPossess(APawn* InPawn) override;
	
	// BPの Event On UnPossess に相当する関数
	virtual void OnUnPossess() override;
	
	
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
	
	// AIPerceptionの更新時に呼ばれるイベント関数
	UFUNCTION()
	void HandlePerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	
	// 各感覚に応じたハンドラ関数
	virtual void HandleSensedSight(AActor* Actor);
	virtual void HandleLostSight(AActor* Actor);
	virtual void HandleSensedSound(FVector Location);
	virtual void HandleSenseDamage(AActor* Actor);
	
private:
	//CanSenseActor関数。「対象のアクターに対する複数ある感覚情報（視覚、聴覚など）の中から、指定した感覚データだけを取り出して、今も検知中かどうかを知りたい」という処理。
	UFUNCTION(BlueprintPure,Category="AIPerception")
	bool CanSenseActor(AActor* Actor,TSubclassOf<class UAISense> Sense,FAIStimulus& OutStimulus);
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
