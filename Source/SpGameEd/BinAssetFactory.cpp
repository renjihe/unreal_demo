#include "BinAssetFactory.h"
#include <Misc/FileHelper.h>
#include <EditorFramework/AssetImportData.h>

UBinAssetFactory::UBinAssetFactory()
{
	// We only want to create assets by importing files.
	// Set this to true if you want to be able to create new, empty Assets from
	// the editor.
	bCreateNew = false;

	bText = false;

	// Allows us to actually use the "Import" button in the Editor for this Asset
	bEditorImport = true;

	// Tells the Editor which file types we can import
	Formats.Add(TEXT("bin;Binary files"));

	// Tells the Editor which Asset type this UFactory can import
	SupportedClass = UBinAsset::StaticClass();
}

UBinAssetFactory::~UBinAssetFactory()
{
}

UObject* UBinAssetFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName
	, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{	
	UBinAsset *result = NewObject<UBinAsset>(InParent, InName, Flags);
	if (result)
	{
		unsigned len = BufferEnd - Buffer;
		result->Content.SetNum(len);
		memcpy(&result->Content[0], Buffer, len);

#if WITH_EDITORONLY_DATA
		if (result->AssetImportData != nullptr)	{
			result->AssetImportData->Update(UFactory::GetCurrentFilename());
		}
#endif	// WITH_EDITORONLY_DATA
		return result;
	}

	return nullptr;
}

/** Returns whether or not the given class is supported by this factory. */
bool UBinAssetFactory::DoesSupportClass(UClass* Class)
{
	return Class == UBinAsset::StaticClass();
}

/** Returns true if this factory can deal with the file sent in. */
bool UBinAssetFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return (Extension == TEXT("bin"));
}

/** Returns the name of the factory for menus */
FText UBinAssetFactory::GetDisplayName() const
{
	return FText();
}

// End UFactory Interface


bool UBinAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	FString path = FAssetTypeActions_BinAsset::ResolveSourceFilePath(Obj);
	
	OutFilenames.Add(path);

	return true;
}

void UBinAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
#if WITH_EDITORONLY_DATA
	if (NewReimportPaths.Num() > 0)
	{
		UBinAsset *asset = Cast<UBinAsset>(Obj);
		if (asset && asset->AssetImportData != nullptr) {
			asset->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
		}
	}	
#endif
}

EReimportResult::Type UBinAssetFactory::Reimport(UObject* Obj)
{
	UBinAsset *asset = Cast<UBinAsset>(Obj);
	if (asset)
	{
		FString path = FAssetTypeActions_BinAsset::ResolveSourceFilePath(Obj);

		TArray<uint8> data;
		if (!FFileHelper::LoadFileToArray(data, *path))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load file '%s' to array"), *path);
			return EReimportResult::Failed;
		}

		asset->Content = data;
		asset->MarkPackageDirty();

		return EReimportResult::Succeeded;
	}

	return EReimportResult::Failed;
}

FString FAssetTypeActions_BinAsset::ResolveSourceFilePath(UObject *binAsset)
{
	FString path;

	if (binAsset)
	{
#if WITH_EDITORONLY_DATA
		UBinAsset *asset = Cast<UBinAsset>(binAsset);
		if (asset && asset->AssetImportData != nullptr) {
			path = asset->AssetImportData->GetFirstFilename();
		}
#endif

		if (path.IsEmpty())
		{
			FString phys_path;
			FString game_path = binAsset->GetPathName(nullptr);

			if (game_path.Find(TEXT("/Game/")) == 0)
			{
				phys_path = FPaths::ProjectContentDir();
				phys_path = FPaths::ConvertRelativePathToFull(phys_path);

				phys_path += &game_path[6];

				int slash = 0;
				if (phys_path.FindLastChar(TEXT('/'), slash))
				{
					phys_path.RemoveAt(slash, phys_path.Len() - slash);

					phys_path += TEXT("/Geometry.bin");
					path = phys_path;
				}
			}
		}		
	}

	return path;
}

void FAssetTypeActions_BinAsset::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (int i = 0; i < TypeAssets.Num(); ++i)
	{
		FString path = ResolveSourceFilePath(TypeAssets[0]);

		OutSourceFilePaths.Add(path);
	}
}
