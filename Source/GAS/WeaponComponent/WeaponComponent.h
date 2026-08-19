// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponComponent.generated.h"

//前方宣言（ビルド時間を短縮するため、クラスの存在だけを宣言します）
class ANexusCharacterBase;

class AWeaponBase;//BP_WeaponBaseの親となるC++クラス

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();
	
	//武器をスポーンして装備（格納）する関数
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void GiveWeapon(TSubclassOf<AWeaponBase> WeaponClass);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// オーナーキャラクターへの参照（OwnerCharacter変数）
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<ANexusCharacterBase> OwnerCharacter;
	
	// 初期装備武器クラスの配列 (StartingWeapons配列変数)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TArray<TSubclassOf<AWeaponBase>> StartingWeapons;
	
	//装備した武器アクタを格納する配列（StowedWeapons配列変数）
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TArray<TObjectPtr<AWeaponBase>> StowedWeapons;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
