// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Props/GraspedProp.h"
#include "AbilityEntity.generated.h"

class USplineMeshComponent;

UENUM(BlueprintType)
namespace EEntityType
{
	enum Type
	{
		Gun,
	};
}

USTRUCT(BlueprintType)
struct SVGAME_API FEntityParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;

	//ÿ�β���Ч������(�ӵ�������������)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PerNum;			

	//�Ƿ�Ϊ����ʩ�ż���
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Infinite;	

	//ÿ��ʹ�ô���
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxInGroup;			

	//ʹ��������InfiniteΪtrueʱ�˲�����Ч��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxInPak;

	//��ɢ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SpreadID;

	//�ζ�����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ShakeID;

	//������
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TraceSocket;

	//���Ŀ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceRadius;

	//Ŀ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int TargetCount;

	//��λ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TEnumAsByte<EPhysicalSurface>, float> PartAdjust;

	//˥��
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damp;

	//˥����Χ
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DampRadius;

	FEntityParam()
	{
		PerNum = 1;
		Infinite = false;
		MaxInGroup = 10;
		MaxInPak = 100;
		SpreadID = 0;
		ShakeID = 0;
		TargetCount = 1;
	}
};

/**
 * 
 */
UCLASS()
class SVGAME_API AAbilityEntity : public AGraspedProp
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		class USplineComponent* SplineComp;

public:
	// Sets default values for this actor's properties
	AAbilityEntity();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	virtual void PickedUpStart(APawn* Pawn) override;

	void PickedUpEnd_Implementation();

	virtual void ThrownAway(bool PutInPackage = false) override;

	virtual void SetCurData(int data1, int data2 = 0.0f) override;
public:
	int GetParamID();

	UFUNCTION(BlueprintCallable, Category = "Ability|Custom")
	FTransform GetShakeOffset();

	void SetShakeOffset(FTransform Offset);
	void SetGoldShakeOffset(FTransform gold);

	float GetSpread();
	void SetSpread(float spread);

	void SetSpreadTime();

	int GetCurNumInGroup();
	int GetCurNumInpak();

	FVector GetTraceWorldLocation_Implementation();

	FRotator GetTraceWorldRotation_Implementation();

public:
	bool TryToConsume();

	void Reload();

	bool NeedReload();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ability)
	TMap<TEnumAsByte<EAbilityBar::Type>, int> Abilitys;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ability)
	TEnumAsByte<EEntityType::Type> EntityType;

	//Gun
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ability)
	int ParamID;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int CurNumInGroup;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int CurNumInPak;

	float CurSpread;
	float SpreadTime;
	float LaggyRecoverTime;

	UPROPERTY(BlueprintReadOnly, Category = Ability)
	FTransform CurrentShakeOffset;

	FTransform GoldShakelOffset;

	bool ApplyShake;
	float ShakeTime;

	UPROPERTY(BlueprintReadOnly, Category = Ability)
	bool ShakeRecoveryFinished;

};
