// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
#include "Engine/HitResult.h"
#else
#include "Engine/EngineTypes.h"
#endif
#include "AC_TargetSystemComponent.generated.h"

class UUserWidget;
class UWidgetComponent;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FConponentOnTargetLockOnOff,AActor*,TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FComponentSetRotation,AActor*,TargetActor,FRotator,ControlRotation);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UAC_TargetSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAC_TargetSystemComponent();
	
//--------------------------------------------------
// 距離判定変数
// --------------------------------------------------
	// ターゲットをロックオンできる最長距離（これより遠い敵は対象外）。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Distance")
	float MinimumDistanceToEnable = 1200.0f;
	
	// ターゲットの検索対象とするアクターのクラス（通常はPawnなど）。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Distance")
	TSubclassOf<AActor> TargetableActor;
	
	// 障害物判定（ライントレース）に用いるコリジョンチャンネル。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Distance")
	TEnumAsByte<ECollisionChannel> TargetableCollisionChannel;
	
	//ロックオン中にプレイヤーキャラクターが敵の方向を見続けるかどうか。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Distance")
	bool bShouldControlRotation = false;
	
	//ロックオン中に通常の視点操作（右スティックやマウス）を無視するか。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Distance")
	bool bIgnoreLookInput = true;
	
	//柱などで視線が遮られた際、即座にロックを外さず猶予を置く時間（秒）。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Distance")
	float BreakLineOfSightDelay = 2.0f;
	
	//スティック等で隣の敵へターゲットを切り替える際の入力しきい値。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Distance")
	float StartRotatingThreshold = 0.85f;
	
//--------------------------------------------------
// UI追従変数
// --------------------------------------------------
	//敵の頭上に表示するカーソルUIを自動生成するかどうか。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Widget")
	bool bShouldDrawLockedOnWidget = true;
	
	//どのウィジェットクラスを追従させるかの設定。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Widget")
	TSubclassOf<UUserWidget> LockedOnWidgetClass;
	
	//ウィジェットのサイズ設定。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Widget")
	float LockedOnWidgetDrawSize;
	
	//どのソケット（例: spine_03）に追従させるかの設定。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Widget")
	FName LockedOnWidgetParentSocket = FName("spine_03");
	
	//追従させるWidgetの位置の設定。
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Widget")
	FVector LockedOnWidgetLocation = FVector(0.0f, 0.0f, 0.0f);
	
//--------------------------------------------------
//カメラ制御変数(敵との距離が近いときに、カメラの上下角度を自動で持ち上げる補正値の設定。)
//--------------------------------------------------	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Camera")
	bool bAdjustPitchBaseOnDistanceToTarget = true;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Camera")
	float MonitorDistanceCoefficient = -0.2f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Camera")
	float MonitorDistanceOffSet = 60.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Camera")
	float MonitorMin = -50.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|Camera")
	float MonitorMax = -20.0f;
	
//--------------------------------------------------
//カメラ制御変数(ターゲット切り替え時にカーソルが吸いつくような操作感を調整するパラメータ。)
//--------------------------------------------------
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|StickyFeelingOnTargetSwitch")
	bool bEnableStickyTarget = false;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|StickyFeelingOnTargetSwitch")
	float AxisMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "TargetSystem|StickyFeelingOnTargetSwitch")
	float StickyRotationThreshold = 30.0f;
	
//--------------------------------------------------
//公開関数
//--------------------------------------------------
	//ロックオンの開始、またはすでにロック中なら解除を行うメインの切り替え関数。
	UFUNCTION(BlueprintCallable, Category = "TargetSystem")
	void TargetActor();
	
	//手動でターゲットを強制解除する関数。
	UFUNCTION(BlueprintCallable, Category = "TargetSystem")
	void TargetLockOff();
	
	//スティックやマウスの入力軸を受け取り、画面の左右にいる別の敵へターゲットを切り替える関数。
	UFUNCTION(BlueprintCallable, Category = "TargetSystem")
	void TargetActorWithAxisInput(float AxisValue);
	
	//現在ロックオン状態にあるかどうかをブール値で返す関数。
	UFUNCTION(BlueprintCallable, Category = "TargetSystem")
	bool GetTargetLockedStatus();
	
	//現在ロックオン状態にあるかどうかをブール値で返す関数。
	UFUNCTION(BlueprintCallable, Category = "TargetSystem")
	bool IsLocked() const;
	
	//現在ターゲットしているアクターのポインタを返す関数。
	UFUNCTION(BlueprintCallable, Category = "TargetSystem")
	AActor* GetLockedOnTargetActor() const;
	
//--------------------------------------------------
//公開関数(デリゲート)
//--------------------------------------------------
	//ロックオンの終了時にBlueprint側へ通知を送るイベントディスパッチャー。
	UPROPERTY(BlueprintAssignable, Category = "TargetSystem")
	FConponentOnTargetLockOnOff OnTargetLockOff;
	
	//ロックオンの開始にBlueprint側へ通知を送るイベントディスパッチャー。
	UPROPERTY(BlueprintAssignable, Category = "TargetSystem")
	FConponentOnTargetLockOnOff OnTargetLockOn;
	
	//カメラの回転処理を独自のロジックでカスタムしたい場合に受け取るイベントディスパッチャー。
	UPROPERTY(BlueprintAssignable, Category = "TargetSystem")
	FComponentSetRotation OnTargetSetRotation;

private:
//--------------------------------------------------
//プライベート変数（内部の状態管理）
//--------------------------------------------------
	//このコンポーネントがアタッチされている親アクター（プレイヤーキャラクターなど）の参照。
	UPROPERTY()
    AActor* OwnerActor;

	//親アクターをPawnとしてキャストした参照（移動制御やコントローラー取得に使用）。
    UPROPERTY()
    APawn* OwnerPawn;

	//操作しているプレイヤーのコントローラーの参照（入力やビューポート計算に使用）。
    UPROPERTY()
    APlayerController* OwnerPlayerController;

	//敵の頭上に生成・表示しているロックオンUIのウィジェットコンポーネント。
    UPROPERTY()
    UWidgetComponent* TargetLockedOnWidgetComponent;

	//現在ロックオンしている対象の敵（アクター）の参照。
    UPROPERTY()
    AActor* LockedOnTargetActor;

	//視線切れの猶予時間や、ターゲット切り替え時のクールダウンを管理するためのタイマーハンドル。
    FTimerHandle LineOfSightBreakTimerHandle;
    FTimerHandle SwitchingTargetTimerHandle;

	//現在、障害物によって視線が遮られている最中かどうかを示すフラグ。
    bool bIsBreakingLineOfSight = false;
	
	//スティック入力などでターゲットを切り替えた直後の、連続切り替え防止中かどうかを示すフラグ。
    bool bIsSwitchingTarget = false;
	
	//現在ターゲットをロックオンしている状態かどうか。
    bool bTargetLocked = false;
	
	//索敵時に最も近い敵までの距離を一時的に保持する変数。
    float ClosestTargetDistance = 0.0f;

	//スティック入力による「ターゲットの切り替え意思」や、スティッキー感（吸い付き）を計算するためのスタック値。
    bool bDesireToSwitch = false;
    float StartRotatingStack = 0.0f;
	
//--------------------------------------------------
//プライベート関数（索敵・判定関連）
//--------------------------------------------------
	//指定されたクラス（敵など）の全アクターをワールド内から検索し、ターゲット可能か精査してリスト化する。
    TArray<AActor*> GetAllActorsOfClass(TSubclassOf<AActor> ActorClass) const;
	
	//プレイヤーの視線やカメラアングルを基準に、指定された左右の角度範囲（RangeMin〜RangeMax）にいる敵を絞り込む。
    TArray<AActor*> FindTargetsInRange(TArray<AActor*> ActorsToLook, float RangeMin, float RangeMax) const;
	
	//候補となった敵の中から、プレイヤーに最も近いアクターを決定する。
    AActor* FindNearestTarget(TArray<AActor*> Actors) const;

	//プレイヤーから敵へ向かってレイ（光線）を飛ばし、間に障害物がないかを判定する。
    bool LineTrace(FHitResult& OutHitResult, const AActor* OtherActor, const TArray<AActor*>& ActorsToIgnore) const;
    bool LineTraceForActor(const AActor* OtherActor, const TArray<AActor*>& ActorsToIgnore) const;

	//敵が障害物に隠れたかを判定し、遅延時間の経過後にロックを強制解除する。
    bool ShouldBreakLineOfSight() const;
    void BreakLineOfSight();

	//敵のワールド座標が、画面（ビューポート）の内側に収まっているかをチェックする。
    bool IsInViewport(const AActor* TargetActor) const;
	
	//プレイヤーと対象アクターとの直線距離を計算する。
    float GetDistanceFromCharacter(const AActor* OtherActor) const;

	//対象アクターがインターフェース（TargetSystemTargetable）を実装しており、かつ現在ターゲット可能かを確認する。
    static bool TargetIsTargetable(const AActor* Actor);
	
//--------------------------------------------------
//プライベート関数（回転・カメラ制御関連）
//--------------------------------------------------
	//ロックオン中にカメラやコントローラーが向くべき理想の回転角度（Pitch/Yawの補正含む）を計算する。
	FRotator GetControlRotationOnTarget(const AActor* OtherActor) const;
	
	//計算した回転角度をコントローラーに適用する（カスタムイベントがバインドされていなければ直接反映）。
	void SetControlRotationOnTarget(AActor* TargetActor) const;
	
	//キャラクターの移動方向への自動回転（bOrientRotationToMovement）をオフにし、コントローラーのYaw回転に追従させる切り替えを行う。
	void ControlRotation(bool ShouldControlRotation) const;

	//カメラやキャラクターの向きを基準にして、対象アクターがどれだけ左右にずれているかの角度を算出する。
	float GetAngleUsingCameraRotation(const AActor* ActorToLook) const;
	float GetAngleUsingCharacterRotation(const AActor* ActorToLook) const;

	//始点から終点へ向かう回転（Rotator）を算出する数学的ヘルパー関数。
	static FRotator FindLookAtRotation(const FVector Start, const FVector Target);

//--------------------------------------------------
//プライベート関数（回転・カメラ制御関連）
//--------------------------------------------------
	//ターゲットした敵のメッシュ（ソケット等）にロックオンUIウィジェットを生成してアタッチする。
	void CreateAndAttachTargetLockedOnWidgetComponent(AActor* TargetActor);
	
	//ロックオン開始時（ウィジェット生成、入力制限、イベント通知）および終了時の処理を一括で行う。
	void TargetLockOn(AActor* TargetToLockOn);
	
	//ターゲット切り替え中のクールダウン状態をリセットする。
	void ResetIsSwitchingTarget();
	
	//スティックの入力軸の値をもとに、ターゲットを切り替える閾値に達しているかを判定する。
	bool ShouldSwitchTargetActor(float AxisValue);
	
	//親Pawnからプレイヤーコントローラーを取得してキャッシュする（ローカルマルチ等への配慮）。
	void SetupLocalPlayerController();
	
protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
};
