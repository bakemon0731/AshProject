// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorPoolSubsystem.h"
#include "GAS/Interface/PoolableActor.h"
#include "Engine/World.h"


AActor* UActorPoolSubsystem::AcquireActor(
	TSubclassOf<AActor> ActorClass, 
	const FTransform& SpawnTransform,
	AActor* Owner, 
	APawn* SpawnInstigator)
{
	if (!ActorClass)
	{
		return nullptr;
	}
	
	//構造体からActorClassを見つけて追加する。
	FPooledActorArray& Pool = Pools.FindOrAdd(ActorClass);
	
	AActor* Actor = nullptr;
	//プールに中身があって、かつ、まだまともなアクタが確保できていない間はループを繰り返し
	while (Pool.Actors.Num() > 0 && !IsValid(Actor))
	{
		Actor = Pool.Actors.Pop();
	}
	
	//プールにアクターがない場合。
	if(!IsValid(Actor))
	{
		//今ゲームが動いているこの世界（World)の情報があるかどうか。
		UWorld* World = GetWorld();
		if (!World)
		{
			return nullptr;
		}
		
		//World情報がある場合、Actorをスポーンさせるパラメータを設定。
		FActorSpawnParameters Params;
		Params.Owner = Owner;
		Params.Instigator = SpawnInstigator;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		//Actorをスポーンさせる。
		Actor = World->SpawnActor<AActor>(ActorClass,SpawnTransform,Params);
	}
	//プールから再利用できるアクタが見つかった時の処理。
	else
	{
		// 新しい所有者（Owner）を設定し直す
		Actor->SetOwner(Owner);
		// 新しい攻撃発生源などの情報（Instigator）を設定し直す
		Actor->SetInstigator(SpawnInstigator);
		// 指定された新しい位置・回転・サイズ（Transform）に瞬間移動させる
		Actor->SetActorTransform(SpawnTransform);
		// 非表示になっていたアクタを、画面に「再表示」する
		Actor->SetActorHiddenInGame(false);
		// オフになっていた物理衝突やアタリ判定（Collision）を「有効」に戻す
		Actor->SetActorEnableCollision(true);
		// 止まっていた毎フレームの更新処理（Tick）を「再開」させる
		Actor->SetActorTickEnabled(true);
	}
	
	//Actorが有効かつ、そのActorにインタフェース（PoolableActor）が実装されているかどうか。
	if (Actor && Actor->GetClass()->ImplementsInterface(UPoolableActor::StaticClass()))
	{
		//有効な場合、インターフェースのOnActivatedFromPool関数を実行。
		IPoolableActor::Execute_OnActivatedFromPool(Actor);
	}
	return Actor;
}

void UActorPoolSubsystem::ReleaseActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	//そのActorにインタフェース（PoolableActor）が実装されているかどうか。
	if (Actor->GetClass()->ImplementsInterface(UPoolableActor::StaticClass()))
	{
		//有効な場合、インターフェースのOnReturnToPool関数を実行。
		IPoolableActor::Execute_OnReturnToPool(Actor);
	}
	
	//アクターの見た目を非表示
	Actor->SetActorHiddenInGame(true);
	//コリジョンを無効化。
	Actor->SetActorEnableCollision(false);
	//Tickを無効化。
	Actor->SetActorTickEnabled(false);
	
	//返却されてきたアクタの正確なクラス（型）を取得。
	FPooledActorArray& Pool = Pools.FindOrAdd(Actor->GetClass());
	//取得したクラス専用のプール（配列）の末尾に、今回使い終わったアクタを Add（追加）。
	Pool.Actors.Add(Actor);
}

void UActorPoolSubsystem::PrewarmPool(TSubclassOf<AActor> ActorClass, int32 Count)
{
	//指定されたクラスが空なら処理を中断。
	if (!ActorClass)
	{
		return;
	}
	
	//Temp という「一時的な配列」を用意し、Count分のメモリ領域を確保（Reserve）して処理を高速化
	TArray<AActor*> Temp;
	Temp.Reserve(Count);
	
	//Count分だけアクタを生成して確保し、AcquireActorを呼び出してスポーンさせる。
	for (int i = 0; i < Count; ++i)
	{
		if (AActor* Actor = AcquireActor(ActorClass,FTransform::Identity))
		{
			//生成されたアクタは、一度一時的な配列（Temp）に保存。
			Temp.Add(Actor);
		}
	}
	
	//生成したアクタをすべてプールにしまい込む。
	for (AActor* Actor : Temp)
	{
		//ReleaseActor関数を呼び出し、休止状態にする。
		ReleaseActor(Actor);
	}
}




