#include "InnoactivePortalStyle.h"
#include "InnoactivePortal.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FInnoactivePortalStyle::StyleInstance = NULL;

void FInnoactivePortalStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FInnoactivePortalStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FInnoactivePortalStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("InnoactivePortalStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FInnoactivePortalStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("InnoactivePortalStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("InnoactivePortal")->GetBaseDir() / TEXT("Resources"));

	Style->Set("InnoactivePortal.PluginCommand", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	Style->Set("InnoactivePortal.PluginCommand.Small", new IMAGE_BRUSH(TEXT("ButtonIcon_20x"), Icon20x20));

	return Style;
}

#undef IMAGE_BRUSH

void FInnoactivePortalStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FInnoactivePortalStyle::Get()
{
	return *StyleInstance;
}