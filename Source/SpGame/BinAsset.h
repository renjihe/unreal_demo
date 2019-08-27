
#pragma once

#include "Object.h"
#include "BinAsset.generated.h"

/**
 * 
 */
UCLASS()
class SPGAME_API UBinAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UAssetImportData* AssetImportData;

	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
#endif
	
	
public:
	UPROPERTY()
	TArray<uint8> Content;

};
