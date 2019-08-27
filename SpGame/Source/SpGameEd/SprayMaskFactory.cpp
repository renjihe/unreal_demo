#include "SprayMaskFactory.h"
#include "SprayMask.h"

USprayMaskFactory::USprayMaskFactory()
{
    // We only want to create assets by importing files.
    // Set this to true if you want to be able to create new, empty Assets from
    // the editor.
    bCreateNew = false;
   
    bText = false;
    
    // Allows us to actually use the "Import" button in the Editor for this Asset
    bEditorImport = true;
    
    // Tells the Editor which file types we can import
    Formats.Add(TEXT("spray;Spray Files"));
    
    // Tells the Editor which Asset type this UFactory can import
    SupportedClass = USprayMask::StaticClass();
}

USprayMaskFactory::~USprayMaskFactory()
{
}

UObject* USprayMaskFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName
	, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
	uint32 w = *(uint32*)(Buffer), h = *(uint32*)(Buffer + 4), size = *(uint32*)(Buffer + 8);
	if (size == w * h * 4 && BufferEnd - Buffer == 12 + size)
	{
		USprayMask *result = NewObject<USprayMask>(InParent, InName, Flags);
		if (result)
		{
			result->Width = w;
			result->Height = h;
			result->Pixels.SetNum(w*h);
			memcpy(&result->Pixels[0], Buffer+12, size);
			return result;
		}
	}

	return nullptr;
}

/** Returns whether or not the given class is supported by this factory. */
bool USprayMaskFactory::DoesSupportClass(UClass* Class)
{
    return false;
}

/** Returns true if this factory can deal with the file sent in. */
bool USprayMaskFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return (Extension == TEXT("spray"));
}

/** Returns the name of the factory for menus */
FText USprayMaskFactory::GetDisplayName() const
{
    return FText();
}

// End UFactory Interface
