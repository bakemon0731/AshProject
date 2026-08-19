// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class GAS : ModuleRules
{
	public GAS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities","GameplayTasks","GameplayTags" });
		
		//GameplayAbilities
		//ゲーム内のキャラクターが使用できる 能力やスキルのシステム を提供します
		//アビリティの定義、実行、キャンセルなどの管理
		//例：攻撃、魔法、ジャンプなどのアクション
		
		//GameplayTags
		//ゲームオブジェクトに タグを付与して分類・識別 するシステムです
		//キャラクターの状態を管理（例：「移動中」「攻撃中」「スタン中」など）
		//アビリティや効果に条件を設定する際に使用

		//GameplayTasks
		//非同期タスク を管理するシステムです
		//複数のアニメーション、サウンド、エフェクトなどを 順序立てて実行
		//タイムラインが必要な複雑なアクションを制御
		
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
