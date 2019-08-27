#pragma once

#include "AbilitiesType.h"

#include "GameFramework/Actor.h"
#include "BarracksActor.generated.h"

UENUM(BlueprintType)
namespace EMonsterType
{
	enum Type
	{
		Unknown,
		Normal,
		Boss,
		MAX
	};
}

UCLASS()
class SVGAME_API ABarracksActor : public AActor
{
	GENERATED_BODY()
public: 
	ABarracksActor();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Barracks)
	bool AutoCreate;
	// Number: How many *** will producted by one time.
	UPROPERTY(EditInstanceOnly, Category = Barracks)
	int32 Number;

	// Frequency: How many seconds will take .
	UPROPERTY(EditInstanceOnly, Category = Barracks)
	float Frequency;

	UPROPERTY(EditInstanceOnly, Category = Barracks)
	TEnumAsByte<EMonsterType::Type> SpawnTeamNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Classes)
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditAnywhere, Category = Barracks)
	int LookId;

	UPROPERTY(EditAnywhere, Category = Barracks)
	int JobId;

	UPROPERTY(EditAnywhere, Category = Barracks)
	FAIExtra AIExtra;
	
public:
	// Called every frame
	virtual void Tick(float deltaTime) override;
	
private:
	float PrevTime;
	void CreateZombie();
};