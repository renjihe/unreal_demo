// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GUI/DlgSingleton.h"
#include "DlgBattleInfo.generated.h"

class UImage;
class UButton;
class UTextBlock;
class UMaterialInterface;
class UUniformGridPanel;
/**
 * 
 */
UCLASS()
class SPGAME_API UDlgBattleInfo : public UDlgSingleton
{
	GENERATED_BODY()
	DECLARE_DLGBASE_INSTANCE(UDlgBattleInfo)

protected:
	virtual void Create() override;
	virtual void Init() override;

public:
	virtual void UpdateFrame(float DeltaSeconds) override;

	void ShowDeadNotice();
	void ShowRebirthNotice();
	void UpdateMPRange(float rate);

	UTextBlock	*BattleTimer;
	UTextBlock	*RebirthTimer;
	UWidget     *DeadCanvas;
	UMaterialInstanceDynamic *MpProgressBar;

	UImage *MiniMap;

	UUniformGridPanel *SprayPanel, *AtkerPanel;
	//const int32 PlayerNums;
	UButton *BtnSpray[4];
	UButton *BtnAtker[4];
	UImage *Attr1, *Attr2, *Attr3;
	UTextBlock	*HpTextBlock, *MpTextBlock;
};
