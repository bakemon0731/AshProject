// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayEffectTypes.h"
#include "Perception/AIPerceptionTypes.h"//AIの感知データ（FAIStimulus）を扱うために必要
#include "GameplayTagContainer.h"
#include "GAS/GamePlayAbilitySystem/Characters/NexusCharacterBase.h"//TeamNumberが定義されている親クラスのヘッダー
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
	
	// ステートをAttackingに変更する関数
	UFUNCTION(BlueprintCallable,Category="State")
	void SetStateAsAttacking(AActor* TargetActor);
	
	// ステートをSeekingに変更する関数
	UFUNCTION(BlueprintCallable,Category="State")
	void SetStateAsSeeking(FVector Location);
	
	// ステートをInvestigatingに変更する関数
	UFUNCTION(BlueprintCallable,Category="State")
	void SetStateAsInvestigating(FVector Location);

	
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
	
	// ステート変更処理を共通化した関数
	UFUNCTION(BlueprintCallable,Category="StateEffect")
	void ChangeStateEffect(TSubclassOf<class UGameplayEffect> NewStateEffect);
	
	// 現在のStateタグを取得する関数
	UFUNCTION(BlueprintCallable,Category="State")
	FGameplayTag GetCurrentState() const;
	
	// チーム判定関数（BPのOn Same Team）
	UFUNCTION(BlueprintCallable,Category="Team")
	bool OnSameTeam(AActor* OtherActor) const;
	
	// AttackTargetを見失った３秒後に呼ばれる関数
	UFUNCTION()
	void SeekAttackTarget();
	
	// 過去に視認していたが、現在は視界から外れて見失った対象（ターゲット）を検知する
	UFUNCTION()
	void CheckIfForgottenSeeActor();
	
	//完全に視界からロストしたアクターのリスト除外とステート初期化を行う
	UFUNCTION()
	void HandleForgotActor (AActor* Actor);
	
	// Set Value as FloatノードのKeyNameの変数
	UPROPERTY(EditDefaultsOnly,Category="Blackboard")
	FName AttackRediusKeyName = "AttackRadius";
	
	// Set Value as FloatノードのKeyNameの変数
	UPROPERTY(EditDefaultsOnly,Category="Blackboard")
	FName DefendRadiusKeyName = "DefendRadius";
	
	// Set Value as ObjectノードのKeyNameの変数
	UPROPERTY(EditDefaultsOnly,Category="Blackboard")
	FName AttackTargetKeyName = "AttackTarget";
	
	// Set Value as VectorノードのKeyNameの変数
	UPROPERTY(EditDefaultsOnly,Category="Blackboard")
	FName PointOfInterestKeyName = "PointOfInterest";
	
	// 付与する GameplayEffect (GE_Passive)
	UPROPERTY(EditDefaultsOnly,Category="StateEffect")
	TSubclassOf<class UGameplayEffect>PassiveStateEffect;
	
	// 付与する GameplayEffect (GE_Attacking)
	UPROPERTY(EditDefaultsOnly,Category="StateEffect")
	TSubclassOf<class UGameplayEffect>AttackingStateEffect;
	
	// 付与する GameplayEffect (GE_Seeking)
	UPROPERTY(EditDefaultsOnly,Category="StateEffect")
	TSubclassOf<class UGameplayEffect>SeekingStateEffect;
	
	// 付与する GameplayEffect (GE_Investigating)
	UPROPERTY(EditDefaultsOnly,Category="StateEffect")
	TSubclassOf<class UGameplayEffect>InvestigatingStateEffect;
	
	// 視覚で捉えたアクターのリスト (BPの Known Seen Actors)
	UPROPERTY(EditDefaultsOnly,Category="AIPerception")
	TArray<AActor*> KnownSeenActors;
	
	// Seekingタイマーのハンドル (BPの Seek Attack Target Timer)
	UPROPERTY(EditDefaultsOnly,Category="AITimer")
	FTimerHandle SeekAttackTargetTimer;
	
	// タイマー制御用のハンドル (BPの Check Forgotten Actor Timer 変数
	UPROPERTY(EditDefaultsOnly,Category="AITimer")
	FTimerHandle  CheckForgottenActorTimer;
	
	//Seekingを始める時間の変数(TimeToSeeAfterLosingSight)
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="AITimer")
	float TimeToSeeAfterLosingSight;
	
	//AttakTarget変数
	UPROPERTY(EditDefaultsOnly,Category="AttackTarget")
	AActor* AttackTarget;
	
	// 現在のStateエフェクトのハンドル (BPの Current State Effect 変数)
	UPROPERTY(EditDefaultsOnly,Category="CurrentState")
	FActiveGameplayEffectHandle CurrentStateEffect;
	
private:
	//CanSenseActor関数。「対象のアクターに対する複数ある感覚情報（視覚、聴覚など）の中から、指定した感覚データだけを取り出して、今も検知中かどうかを知りたい」という処理。
	UFUNCTION(BlueprintPure,Category="AIPerception")
	bool CanSenseActor(AActor* Actor,TSubclassOf<class UAISense> Sense,FAIStimulus& OutStimulus);
	
	
	
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
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
