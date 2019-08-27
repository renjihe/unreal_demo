#include "SvGame.h"
#include "BTTask_BlueprintBaseSV.h"

void UBTTask_BlueprintBaseSV::SetOwner(AActor* owner)
{
	Super::SetOwner(owner);

	BtAIOwner = Cast<AAIController>(owner);
}
