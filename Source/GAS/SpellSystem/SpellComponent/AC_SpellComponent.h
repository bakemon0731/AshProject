// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"
#include "GAS/SpellSystem/SpellDataAsset.h"
#include "AC_SpellComponent.generated.h"

//イベントディスパッチャーの宣言
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquippedSpellsChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_API UAC_SpellComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAC_SpellComponent();
	
	// 習得済み魔法（覚えている魔法）。BPのクラスデフォルトで編集
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= "Spells")
	TArray<TObjectPtr<USpellDataAsset>> KnownSpells;

	// スロット0〜4の中身。付け替え対象
	UPROPERTY(ReplicatedUsing=OnRep_EquippedSpellSlots, BlueprintReadOnly, Category= "Spells")
	TArray<TObjectPtr<USpellDataAsset>> EquippedSpellSlots;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Spells")
	int32 NumSpellSlots = 5;
	
	//イベントディスパッチャーの宣言変数
	UPROPERTY(BlueprintAssignable,Category= "Spells")
	FOnEquippedSpellsChanged OnEquippedSpellsChanged;
	
public:
	
	// PossessedBy等、Owner初期化後にCharacter側から呼ぶ
	UFUNCTION(BlueprintCallable, Category= "Spells")
	void InitializeKnownSpells(UAbilitySystemComponent* InASC);
	
	// GA_CastSelectedSpell から呼ばれる
	UFUNCTION(BlueprintCallable, Category= "Spells")
	bool ActivateSpellIndex(int32 SpellIndex);
	
	// UIから呼ぶ
	UFUNCTION(BlueprintCallable, Category= "Spells")
	void EquipSpellToSlot(int32 SlotIndex,USpellDataAsset* Spell);
	
	UFUNCTION(BlueprintPure, Category= "Spells")
	USpellDataAsset* GetEquippedSpellAtSlot(int32 SlotIndex) const;
	
	UFUNCTION(BlueprintPure, Category= "Spells")
	int32 GetNumSpellSlots() const { return NumSpellSlots; }
	
	//装備中の魔法をスペルブックから非表示にする
	UFUNCTION(BlueprintPure, Category= "Spells")
	bool IsSpellEquippedInAnySlot(USpellDataAsset* Spell) const;
	
protected:
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;
	
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>,FGameplayAbilitySpecHandle> SpellHandleMap;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	//ServerRPC
	UFUNCTION(Server, Reliable)
	void Server_EquipSpellToSlot(int32 SlotIndex,USpellDataAsset* Spell);
	
	UFUNCTION()
	void OnRep_EquippedSpellSlots();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
