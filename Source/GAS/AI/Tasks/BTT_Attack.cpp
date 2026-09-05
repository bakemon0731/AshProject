// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_Attack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/GameplayAbilitySystem/Abilities/NexusGameplayAbility.h"

// コンストラクタ：ノードの初期設定
UBTT_Attack::UBTT_Attack()
{
    // ビヘイビアツリーの画面上に表示されるノード名を指定
    NodeName = "Attack Task (C++)";
}

// タスク実行時のメイン処理（BPの Receive Execute に相当）
EBTNodeResult::Type UBTT_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    //  AIが操作しているPawn（キャラクター）を取得（取得失敗時は即座にタスク失敗）
    APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!ControlledPawn) return EBTNodeResult::Failed;

    // PawnからGASの能力管理コンポーネント(ASC)を取得
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn);
    // ASCが存在しない、または実行対象のアビリティクラスが未設定の場合は失敗
    if (!ASC || !AbilityToActivate) return EBTNodeResult::Failed;

    // タスク完了時（OnMyAbilityEnded）にビヘイビアツリーへ完了報告をするため OwnerComp を参照保持
    CachedOwnerComp = &OwnerComp;

    //  指定されたアビリティクラスから、ASC内に登録されているスペック（仕様データ）をピンポイントで検索
    FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityToActivate);
    if (!Spec) return EBTNodeResult::Failed;

    // アビリティを発動（クールダウンやコスト不足等で発動できなかった場合は失敗）
    if (!ASC->TryActivateAbility(Spec->Handle)) return EBTNodeResult::Failed;

    // 発動した Spec から、現在動いているアビリティの実体（Primary Instance）を直接取得
    if (UGameplayAbility* ActiveInstance = Spec->GetPrimaryInstance())
    {
        // 自作アビリティクラス（UNexusGameplayAbility）へ安全にキャスト
        if (UNexusGameplayAbility* ExAbility = Cast<UNexusGameplayAbility>(ActiveInstance))
        {
            //  アビリティ終了イベントに自身の OnMyAbilityEnded 関数をバインド（BPの Bind Event）
            ExAbility->OnAbilityEnded.AddDynamic(this, &UBTT_Attack::OnMyAbilityEnded);
            
            // アビリティ完了を待つため、タスクを「実行中」のまま一時保留にする
            return EBTNodeResult::InProgress;
        }
    }

    // インスタンス取得やキャストに失敗した場合はタスク失敗
    return EBTNodeResult::Failed;
}

// アビリティ終了時に呼び出されるコールバック関数
void UBTT_Attack::OnMyAbilityEnded(UGameplayAbility* Ability)
{
    // 二重発火やメモリ残存を防ぐため、バインドしたイベントを解除
    if (UNexusGameplayAbility* ExAbility = Cast<UNexusGameplayAbility>(Ability))
    {
        ExAbility->OnAbilityEnded.RemoveDynamic(this, &UBTT_Attack::OnMyAbilityEnded);
    }

    // 保持しておいた OwnerComp が破棄されず安全に存在するか確認
    if (CachedOwnerComp.IsValid())
    {
        // ビヘイビアツリーに「タスク成功」を通知し、次のノードへ移行させる（BPの Finish Execute）
        FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
    }
}