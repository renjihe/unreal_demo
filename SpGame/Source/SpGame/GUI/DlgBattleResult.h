// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GUI/DlgSingleton.h"
#include "DlgBattleResult.generated.h"

struct FNumberFormattingOptions;
class UImage;
class UTextureRenderTarget2D;
class UTextBlock;
class UProgressBar;
class UButton;

/**
 * 
 */
UCLASS()
class SPGAME_API UDlgBattleResult : public UDlgSingleton
{
	GENERATED_BODY()
	DECLARE_DLGBASE_INSTANCE(UDlgBattleResult)
protected:
	virtual void Create() override;
	virtual void Init() override;
public:
	virtual void UpdateFrame(float DeltaSeconds) override;
	void UpdateScore(float atkerScore, float deferScore);

	void RenderCharacter();
	void ShowResult();
	inline int32 GetWinner();
	inline float CalcCameraPos(int32 playerNumber);

	UFUNCTION()
	void SysSettingClicked();
	
	UImage *ImageCharacter;
	UTextureRenderTarget2D *CharacterRenderTarget;

	UImage *MapResult;
	UTextureRenderTarget2D *MapRenderTarget;
	
	UTextBlock *TextAtkerPercent, *TextDeferPercent;
	UTextBlock *TextAtkerResult, *TextDeferResult;
	UTextBlock *TextAtkerExp, *TextDeferExp;
	UImage	   *ProgressBar;
	UMaterialInstanceDynamic *ProgressMat;

	UButton	   *BtnSysSetting;
	FNumberFormattingOptions Formate;

	float AtkerPercent, DeferPercent;
	float AtkerScore, DeferScore;

	float Time;
	const float During = 2.5;
	const float T1 = During * 0.45;
	const float T2 = During * 0.94;
	const float T3 = During;

	float HalfofScore;
	float HalfofPercent;

	float CalcRange(float total, float half, float time);
};