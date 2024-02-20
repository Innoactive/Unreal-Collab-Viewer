// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "InnoactivePortal.h"
#include "InnoactivePortalCommands.h"
#include "InnoactivePortalSettings.h"
#include "InnoactivePortalStyle.h"
#include "ISettingsModule.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "Modules/ModuleManager.h"

static const FString STARTUP_BLUEPRINT = "/InnoactivePortal/StreamerWidget.StreamerWidget";
#define LOCTEXT_NAMESPACE "FInnoactivePortalModule"

void FInnoactivePortalModule::StartupModule()
{
	// register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	FInnoactivePortalStyle::Initialize();
	FInnoactivePortalStyle::ReloadTextures();

	FInnoactivePortalCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FInnoactivePortalCommands::Get().PluginCommand,
		FExecuteAction::CreateRaw(this, &FInnoactivePortalModule::AddWidget));


	if (SettingsModule != nullptr)
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "InnoactivePortal",
			LOCTEXT("InnoactivePortalSettingsName", "Innoactive Portal"),
			LOCTEXT("InnoactivePortalSettingsDescription", "Project settings for Innoactive Portal plugin"),
			GetMutableDefault<UInnoactivePortalSettings>()
		);
	}

	#if ENGINE_MAJOR_VERSION >= 5
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		FToolMenuSection& Section = ToolbarMenu->AddSection("InnoactivePortalComboButton");
		FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			FInnoactivePortalCommands::Get().PluginCommand,
			LOCTEXT("LevelEditorToolbarInnoactivePortalButtonLabel", "Innoactive Portal"),
			LOCTEXT("LevelEditorToolbarInnoactivePortalButtonTooltip", "Innoactive Portal"),
			FSlateIcon(FInnoactivePortalStyle::GetStyleSetName(), "InnoactivePortal.PluginCommand")
		));
		Entry.SetCommandList(PluginCommands);
	#else
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender());
		ToolbarExtender->AddToolBarExtension("Compile", EExtensionHook::After, PluginCommands,
			FToolBarExtensionDelegate::CreateRaw(this, &FInnoactivePortalModule::AddWidget));
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	#endif

}

void FInnoactivePortalModule::AddWidget()
{
	const FSoftObjectPath widgetAssetPath(STARTUP_BLUEPRINT);

	UObject* widgetAssetLoaded = widgetAssetPath.TryLoad();
	if (widgetAssetLoaded == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Missing Expected widget class at : %s"), *STARTUP_BLUEPRINT);
		return;
	}

	UEditorUtilityWidgetBlueprint* widget = Cast<UEditorUtilityWidgetBlueprint>(widgetAssetLoaded);
	if (widget == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("Couldn't cast %s to UEditorUtilityWidgetBlueprint"), *STARTUP_BLUEPRINT);
		return;
	}

	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	EditorUtilitySubsystem->SpawnAndRegisterTab(widget);
}

void FInnoactivePortalModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// unregister settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "InnoactivePortal");
	}

	FInnoactivePortalStyle::Shutdown();

	FInnoactivePortalCommands::Register();
}


IMPLEMENT_MODULE(FInnoactivePortalModule, InnoactivePortal)

#undef LOCTEXT_NAMESPACE