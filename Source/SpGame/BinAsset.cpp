#include "BinAsset.h"
#if WITH_EDITORONLY_DATA
#	include <EditorFramework/AssetImportData.h>
#endif

UBinAsset::UBinAsset(const FObjectInitializer &objIniter)
	: Super(objIniter)
{
#if WITH_EDITORONLY_DATA
	AssetImportData = CreateEditorOnlyDefaultSubobject<UAssetImportData>(TEXT("AssetImportData"));
#endif
}


#if WITH_EDITORONLY_DATA
void UBinAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (AssetImportData)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(OutTags);
}
#endif
