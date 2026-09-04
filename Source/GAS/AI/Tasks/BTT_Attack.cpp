// Fill out your copyright notice in the Description page of Project Settings.

#include "BTT_Attack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/GameplayAbilitySystem/Abilities/NexusGameplayAbility.h"

UBTT_Attack::UBTT_Attack()
{
    NodeName = "Attack Task (C++)";
}

EBTNodeResult::Type UBTT_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    APawn* ControlledPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!ControlledPawn) return EBTNodeResult::Failed;

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn);
    if (!ASC || !AbilityToActivate) return EBTNodeResult::Failed;

    CachedOwnerComp = &OwnerComp;

    // アビリティスペックの検索と発動
    FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityToActivate);
    if (!Spec) return EBTNodeResult::Failed;

    bool bSuccess = ASC->TryActivateAbility(Spec->Handle);
    if (!bSuccess) return EBTNodeResult::Failed;

    // 修正: FGameplayAbilitySpec の配列としてループを回す
    for (const FGameplayAbilitySpec& AbilitySpec : ASC->GetActivatableAbilities())
    {
        if (AbilitySpec.Ability && AbilitySpec.Ability->IsA(AbilityToActivate) && AbilitySpec.IsActive())
        {
            // インスタンス化されたアビリティを取得
            if (UGameplayAbility* ActiveInstance = AbilitySpec.GetPrimaryInstance())
            {
                if (UNexusGameplayAbility* ExAbility = Cast<UNexusGameplayAbility>(ActiveInstance))
                {
                    ExAbility->OnAbilityEnded.AddDynamic(this, &UBTT_Attack::OnMyAbilityEnded);
                    return EBTNodeResult::InProgress;
                }
            }
        }
    }

    return EBTNodeResult::Failed;
}

void UBTT_Attack::OnMyAbilityEnded(UGameplayAbility* Ability)
{
    if (UNexusGameplayAbility* ExAbility = Cast<UNexusGameplayAbility>(Ability))
    {
        ExAbility->OnAbilityEnded.RemoveDynamic(this, &UBTT_Attack::OnMyAbilityEnded);
    }

    if (CachedOwnerComp.IsValid())
    {
        // 修正: 混入していた [&] を削除
        FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
    }
}