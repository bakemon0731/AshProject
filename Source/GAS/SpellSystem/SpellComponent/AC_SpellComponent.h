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
	
	//Spellスロット数
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Spells")
	int32 NumSpellSlots = 10;
	
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
	
	//(読み取り専用）
	UFUNCTION(BlueprintPure, Category= "Spells")
	USpellDataAsset* GetEquippedSpellAtSlot(int32 SlotIndex) const;
	
	UFUNCTION(BlueprintPure, Category= "Spells")
	int32 GetNumSpellSlots() const { return NumSpellSlots; }
	
	//装備中の魔法をスペルブックから非表示にする（読み取り専用）
	UFUNCTION(BlueprintPure, Category= "Spells")
	bool IsSpellEquippedInAnySlot(USpellDataAsset* Spell) const;
	
	//サーバーへ選択結果を明示的に送るServerRPC
	UFUNCTION(Server, Reliable, BlueprintCallable, Category= "AbilitySystem")
	void Server_SetSelectedSpellIndex(int32 NewIndex);
	
	//装備しているSpellSlotからデータアセットのCost変数を出してTotalで返す関数。（読み取り専用）
	UFUNCTION(BlueprintPure, Category= "Spells")
	int32 GetUsedMemoryCapacity() const;
	
	//MaxMemoryCapacity変数の値を返す。（読み取り専用）
	UFUNCTION(BlueprintPure, Category= "Spells")
	int32 GetReturnMaxMemoryCapacity() const {return GetMaxMemoryCapacity(); }
	
	// 「この魔法をこのスロットに入れられるか」を事前判定する関数。（読み取り専用）
	UFUNCTION(BlueprintPure, Category= "Spells")
	bool CanEquipSpellToSlot(int32 SlotIndex,USpellDataAsset* Spell) const;
	
	//コスト順に並べ替えられた、習得済みの魔法一覧を取得する関数。（読み取り専用、ソート）
	UFUNCTION(BlueprintPure, Category= "Spells")
	TArray<USpellDataAsset*> GetKnownSpellsSortedByCost() const;
	
	//このTierに属するKnownSpellsのうち、装備されていないものが何個あるかをデータから直接数える関数
	UFUNCTION(BlueprintCallable, Category= "Spells")
	int32 GetUnequippedSpellCountByCost(int32 Cost) const;
	
	UFUNCTION(BlueprintCallable, Category= "Spells")
	void SwapEquippedSpells(int32 SlotIndexA,int32 SlotIndexB);

	UFUNCTION(BlueprintPure, Category = "Spells")
	int32 GetMaxMemoryCapacity() const;
	
protected:
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;
	
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>,FGameplayAbilitySpecHandle> SpellHandleMap;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= "AbilitySystem")
	int32 SelectedSpellIndex;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	//ServerRPC
	UFUNCTION(Server, Reliable)
	void Server_EquipSpellToSlot(int32 SlotIndex,USpellDataAsset* Spell);
	
	UFUNCTION(Server, Reliable)
	void Server_SwapEquippedSpells(int32 SlotIndexA,int32 SlotIndexB);
	
	UFUNCTION()
	void OnRep_EquippedSpellSlots();

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
