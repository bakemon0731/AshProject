// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GAS/Interface/PoolableActor.h"
#include "Iris/ReplicationSystem/ReplicationSystemTypes.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UArrowComponent;
class UProjectileMovementComponent;
class UGameplayEffect;

/**
* * BP_Projectile_Base のC++移植版。
 * ・Speed / TargetLocation / EffectSpecHandle / Instigator は InitializeAndFire() 経由でGAから設定する
 * ・ヒット/オーバーラップ時のGameplayCue実行・Effect適用ロジックを保持
 * ・DestroyActorの代わりにActorPoolSubsystemへ返却する
 *
 * ・見た目(コリジョンプロファイル、GameplayCueタグ、Debuffクラス等)はBP側の
 * 「クラスのデフォルト」で調整できるよう、EditDefaultsOnlyで公開しておく。
 */

UCLASS()
class GAS_API AProjectileBase : public AActor, public IPoolableActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectileBase();

	//発射物のSpeed値を取得する変数。
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Projectile|FireSetup")
	float Speed = 2000.0f;
	
	//ターゲットの位置を取得する変数。
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Projectile|FireSetup")
	FVector TargetLocation = FVector::ZeroVector;
	
	//ダメージエフェクトを与えるスペックハンドルを取得する変数。
	UPROPERTY(BlueprintReadWrite, Category = "Projectile|FireSetup")
	FGameplayEffectSpecHandle EffectSpecHandle;
	
	//発射物を生成したアクターを取得する変数。
	UPROPERTY(BlueprintReadWrite, Category = "Projectile|FireSetup")
	TObjectPtr<AActor> ProjectileInstigator = nullptr;
	
	// 何にも当たらなかった場合、この秒数が経過したら自動的にプールへ返却する(0以下で無効)
	UPROPERTY(BlueprintReadWrite, Category = "Projectile|FireSetup")
	float MaxLifetime = 5.f;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Projectile")
	TObjectPtr<USphereComponent> SphereCollision;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category = "Projectile")
	TObjectPtr<UArrowComponent> Arrow;
	
	UPROPERTY(visibleAnywhere,BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
	
	//スポーン時のGameplayCue
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category = "Projectile|GameplayCue")
	FGameplayTag GameplayCueSpawn;
	
	//インパクト時のGameplayCue
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category = "Projectile|GameplayCue")
	FGameplayTag GameplayCueImpact;
	
	//適用するデバフのGameplayEffect
	UPROPERTY(BlueprintReadWrite, Category = "Projectile|Debuff")
	TSubclassOf<UGameplayEffect> DebuffToApply;
	
	//IPoolableActorインターフェースのオーバーライド(通知を受け取ると自動で実行される（イベント関数）)
	virtual void OnActivatedFromPool_Implementation()override;
	virtual void OnReturnToPool_Implementation()override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//発射物を即座に発射する関数。
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitializeAndFire(
		float InSpeed, 
		FVector InTargetLocation, 
		const FGameplayEffectSpecHandle& InEffectSpecHandle,
		AActor* InInstigator
		);
	
	/**
	 * プールから取得されC++側の初期化(状態リセット)が完了した直後に呼ばれる。
	 * Niagara等の付随エフェクトのActivate/Deactivateなど「見た目」に関する処理はここをBPでオーバーライドして実装する。
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Pooling", meta = (DisplayName = "On Pool Activated"))
	void OnPoolActivated();
	
	// プールに返却されC++側の後片付けが完了した直後に呼ばれる。用途はOnPoolActivatedの逆。
	UFUNCTION(BlueprintImplementableEvent, Category = "Projectile|Pooling", meta = (DisplayName = "On Pool Deactivated"))
	void OnPoolDeactivated();
 
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnSphereHit(
		UPrimitiveComponent* HitComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
		);
	
	UFUNCTION()
	void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);
	
	void ApplyEffectToActor(AActor* TargetActor);
	
	//インパクト時のGC実行関数。NetMulticast化
	UFUNCTION(NetMulticast,Unreliable)
	void ExecuteImpactCue();
	
	//スポーン時のGC実行関数。NetMulticast化
	UFUNCTION(NetMulticast,Unreliable)
	void ExecuteSpawnCue();
	
	//インパクト時にActorPoolSubsystemのReleaseActor関数を呼ぶ。
	void ReturnToPoolOrDestroy();
	
	// MaxLifetime経過時に自動でプール返却するためのタイマー
	FTimerHandle LifeTimeTimerHandle;
	
	void ResolveImpact(AActor* OtherActor);
	
	// Hit/Overlap両方から呼ばれても衝突処理(ダメージ適用・プール返却)を一度しか実行しないためのフラグ
	bool bHasImpacted = false;
};
