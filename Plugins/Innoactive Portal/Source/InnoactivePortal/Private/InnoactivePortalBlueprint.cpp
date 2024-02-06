// Fill out your copyright notice in the Description page of Project Settings.

#include "InnoactivePortalBlueprint.h"
#include "Interfaces/IPluginManager.h"
#include "CoreUObject.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "Engine/GameEngine.h"
#include "EngineGlobals.h"
#include "SlateCore.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "IUATHelperModule.h"
#include "PlatformInfo.h"
#include "InstalledPlatformInfo.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Settings/PlatformsMenuSettings.h"
#include "Interfaces/IMainFrameModule.h"
#include "Interfaces/IProjectTargetPlatformEditorModule.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"
#include "EditorAnalytics.h"
#include "UnrealEdGlobals.h"
#include "FileHelpers.h"
#include "GameMapsSettings.h"
#include "GameProjectGenerationModule.h"
#include "Editor/UnrealEdEngine.h"
#include "DerivedDataCacheInterface.h"
#include "InnoactivePortalLogCategory.h"
#include "InnoactivePortalPythonBridge.h"

#define LOCTEXT_NAMESPACE "UInnoactivePortalBlueprintClass"

bool UInnoactivePortalBlueprintClass::IsInitialized()
{
	return UInnoactivePortalPythonBridge::Get() != nullptr;
}

void UInnoactivePortalBlueprintClass::Authenticate(const FPostAuthentication& PostAuthentication)
{
	UInnoactivePortalPythonBridge* bridge = UInnoactivePortalPythonBridge::Get();

	if (bridge != nullptr)
	{
		bridge->Authenticate(PostAuthentication);
	}
}

void UInnoactivePortalBlueprintClass::ValidateCurrentToken(const FString& AccessToken, const FString& RefreshToken, const int32 OrganizationId, const FPostAuthentication& PostAuthentication)
{
	UInnoactivePortalPythonBridge* bridge = UInnoactivePortalPythonBridge::Get();

	if (bridge != nullptr)
	{
		bridge->ValidateCurrentToken(AccessToken, RefreshToken, OrganizationId, PostAuthentication);
	}
}

void UInnoactivePortalBlueprintClass::PublishAuthenticationEvent(const FPostAuthentication& PostAuthentication, const FString AccessToken, const FString RefreshToken, const int32 OrganizationId)
{
	FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&]()
		{
			PostAuthentication.ExecuteIfBound(AccessToken, RefreshToken, OrganizationId);
		}, TStatId(), NULL, ENamedThreads::GameThread);

	FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task);
}

void UInnoactivePortalBlueprintClass::OpenDirectoryDialog(const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName) {
    void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform)
    {
        uint32 SelectionFlag = 1; //A value of 0 represents single file selection while a value of 1 represents multiple file selection
        DesktopPlatform->OpenDirectoryDialog(ParentWindowPtr, DialogTitle, DefaultPath, OutFolderName);
    }
}

FString GetCookingOptionalParams()
{
	FString OptionalParams;
	const UProjectPackagingSettings* const PackagingSettings = GetDefault<UProjectPackagingSettings>();

	if (PackagingSettings->bSkipEditorContent)
	{
		OptionalParams += TEXT(" -SkipCookingEditorContent");
	}

	if (FDerivedDataCacheInterface* DDC = GetDerivedDataCache())
	{
		OptionalParams += FString::Printf(TEXT(" -ddc=%s"), DDC->GetGraphName());
	}

	return OptionalParams;
}

/**
 * Gets compilation flags for UAT for this system.
 */
const TCHAR* GetUATCompilationFlags()
{
	// We never want to compile editor targets when invoking UAT in this context.
	// If we are installed or don't have a compiler, we must assume we have a precompiled UAT.
	return TEXT("-nocompileeditor");
}

void AddMessageLog(const FText& Text, const FText& Detail, const FString& TutorialLink, const FString& DocumentationLink)
{
	TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Error);
	Message->AddToken(FTextToken::Create(Text));
	Message->AddToken(FTextToken::Create(Detail));
	if (!TutorialLink.IsEmpty())
	{
		Message->AddToken(FTutorialToken::Create(TutorialLink));
	}
	if (!DocumentationLink.IsEmpty())
	{
		Message->AddToken(FDocumentationToken::Create(DocumentationLink));
	}
	FMessageLog MessageLog("PackagingResults");
	MessageLog.AddMessage(Message);
	MessageLog.Open();
}

// Adapted from https://github.com/metaworking/channeld-ue-plugin/blob/ue-5.1/Source/ChanneldEditor/ChanneldEditorSubsystem.cpp
void UInnoactivePortalBlueprintClass::PackageProject(const FName InPlatformInfoName, const FPostPackageProject& PostPackageProject, FString& OutFolderPath)
{
	GUnrealEd->CancelPlayingViaLauncher();
	const bool bPromptUserToSave = false;
	const bool bSaveMapPackages = true;
	const bool bSaveContentPackages = true;
	const bool bFastSave = false;
	const bool bNotifyNoPackagesSaved = false;
	const bool bCanBeDeclined = false;
	FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave,
		bNotifyNoPackagesSaved, bCanBeDeclined);

	// does the project have any code?
	FGameProjectGenerationModule& GameProjectModule = FModuleManager::LoadModuleChecked<
		FGameProjectGenerationModule>(
			TEXT("GameProjectGeneration"));
	bool bProjectHasCode = GameProjectModule.Get().ProjectHasCodeFiles();

#if ENGINE_MAJOR_VERSION >= 5
#if ENGINE_MINOR_VERSION >= 2
	const PlatformInfo::FTargetPlatformInfo* PlatformInfo = nullptr;
	if (FApp::IsInstalled())
	{
		PlatformInfo = PlatformInfo::FindPlatformInfo(InPlatformInfoName);
	}
	else
	{
		PlatformInfo = PlatformInfo::FindPlatformInfo(GetMutableDefault<UPlatformsMenuSettings>()->GetTargetFlavorForPlatform(InPlatformInfoName));
	}
#else
	const PlatformInfo::FTargetPlatformInfo* PlatformInfo = PlatformInfo::FindPlatformInfo(InPlatformInfoName);
#endif
#else
	const PlatformInfo::FPlatformInfo* const PlatformInfo = PlatformInfo::FindPlatformInfo(InPlatformInfoName);
#endif
	check(PlatformInfo);

	//	NOTE: cannot find BinaryFolderName from PlatformInfo on UE5
#if ENGINE_MAJOR_VERSION < 5
	if (FInstalledPlatformInfo::Get().IsPlatformMissingRequiredFile(PlatformInfo->BinaryFolderName))
	{
		if (!FInstalledPlatformInfo::OpenInstallerOptions())
		{
			FMessageDialog::Open(EAppMsgType::Ok,
				LOCTEXT("MissingPlatformFilesPackage",
					"Missing required files to package this platform."));
		}
		PostPackageProject.ExecuteIfBound(false);
		return;
	}
#endif

	if (UGameMapsSettings::GetGameDefaultMap().IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("MissingGameDefaultMap",
				"No Game Default Map specified in Project Settings > Maps & Modes."));
		PostPackageProject.ExecuteIfBound(false);
		return;
	}

	//	NOTE: Cannot find SDKStatus in UE5
#if ENGINE_MAJOR_VERSION < 5
	if (PlatformInfo->SDKStatus == PlatformInfo::EPlatformSDKStatus::NotInstalled || (bProjectHasCode &&
		PlatformInfo->bUsesHostCompiler && !FSourceCodeNavigation::IsCompilerAvailable()))
	{
		IMainFrameModule& MainFrameModule = FModuleManager::GetModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		MainFrameModule.BroadcastMainFrameSDKNotInstalled(PlatformInfo->TargetPlatformName.ToString(),
			PlatformInfo->SDKTutorial);
		TArray<FAnalyticsEventAttribute> ParamArray;
		ParamArray.Add(FAnalyticsEventAttribute(TEXT("Time"), 0.0));
		FEditorAnalytics::ReportEvent(TEXT("Editor.Package.Failed"), PlatformInfo->TargetPlatformName.ToString(),
			bProjectHasCode, EAnalyticsErrorCodes::SDKNotFound, ParamArray);
		PostPackageProject.ExecuteIfBound(false);
		return;
	}
#endif

	UProjectPackagingSettings* PackagingSettings = Cast<UProjectPackagingSettings>(
		UProjectPackagingSettings::StaticClass()->GetDefaultObject());

	// Force the server build target
	if (!PackagingSettings->BuildTarget.EndsWith("Server"))
	{
		PackagingSettings->BuildTarget.Append("Server");
		PackagingSettings->SaveConfig();
	}

	const UProjectPackagingSettings::FConfigurationInfo& ConfigurationInfo =
		UProjectPackagingSettings::ConfigurationInfo[(int)PackagingSettings->BuildConfiguration];
	bool bAssetNativizationEnabled = (PackagingSettings->BlueprintNativizationMethod !=
		EProjectPackagingBlueprintNativizationMethod::Disabled);

#if ENGINE_MAJOR_VERSION >= 5
	FName TargetPlatformName = PlatformInfo->Name;
#else
	FName TargetPlatformName = PlatformInfo->TargetPlatformName;
#endif

	const ITargetPlatform* const Platform = GetTargetPlatformManager()->FindTargetPlatform(
		TargetPlatformName.ToString());
	{
		if (Platform)
		{
			FString NotInstalledTutorialLink;
			FString DocumentationLink;
			FText CustomizedLogMessage;

			int32 Result = Platform->CheckRequirements(bProjectHasCode, ConfigurationInfo.Configuration,
				bAssetNativizationEnabled, NotInstalledTutorialLink,
				DocumentationLink, CustomizedLogMessage);

			// report to analytics
			FEditorAnalytics::ReportBuildRequirementsFailure(
				TEXT("Editor.Package.Failed"), TargetPlatformName.ToString(), bProjectHasCode,
				Result);

			// report to main frame
			bool UnrecoverableError = false;

			// report to message log
			if ((Result & ETargetPlatformReadyStatus::SDKNotFound) != 0)
			{
				AddMessageLog(
					LOCTEXT("SdkNotFoundMessage", "Software Development Kit (SDK) not found."),
					CustomizedLogMessage.IsEmpty()
					? FText::Format(LOCTEXT("SdkNotFoundMessageDetail",
						"Please install the SDK for the {0} target platform!"), Platform->
						DisplayName())
					: CustomizedLogMessage,
					NotInstalledTutorialLink,
					DocumentationLink
				);
				UnrecoverableError = true;
			}

			if ((Result & ETargetPlatformReadyStatus::LicenseNotAccepted) != 0)
			{
				AddMessageLog(
					LOCTEXT("LicenseNotAcceptedMessage", "License not accepted."),
					CustomizedLogMessage.IsEmpty()
					? LOCTEXT("LicenseNotAcceptedMessageDetail",
						"License must be accepted in project settings to deploy your app to the device.")
					: CustomizedLogMessage,
					NotInstalledTutorialLink,
					DocumentationLink
				);

				UnrecoverableError = true;
			}

			if ((Result & ETargetPlatformReadyStatus::ProvisionNotFound) != 0)
			{
				AddMessageLog(
					LOCTEXT("ProvisionNotFoundMessage", "Provision not found."),
					CustomizedLogMessage.IsEmpty()
					? LOCTEXT("ProvisionNotFoundMessageDetail",
						"A provision is required for deploying your app to the device.")
					: CustomizedLogMessage,
					NotInstalledTutorialLink,
					DocumentationLink
				);
				UnrecoverableError = true;
			}

			if ((Result & ETargetPlatformReadyStatus::SigningKeyNotFound) != 0)
			{
				AddMessageLog(
					LOCTEXT("SigningKeyNotFoundMessage", "Signing key not found."),
					CustomizedLogMessage.IsEmpty()
					? LOCTEXT("SigningKeyNotFoundMessageDetail",
						"The app could not be digitally signed, because the signing key is not configured.")
					: CustomizedLogMessage,
					NotInstalledTutorialLink,
					DocumentationLink
				);
				UnrecoverableError = true;
			}

			if ((Result & ETargetPlatformReadyStatus::ManifestNotFound) != 0)
			{
				AddMessageLog(
					LOCTEXT("ManifestNotFound", "Manifest not found."),
					CustomizedLogMessage.IsEmpty()
					? LOCTEXT("ManifestNotFoundMessageDetail",
						"The generated application manifest could not be found.")
					: CustomizedLogMessage,
					NotInstalledTutorialLink,
					DocumentationLink
				);
				UnrecoverableError = true;
			}

			if ((Result & ETargetPlatformReadyStatus::RemoveServerNameEmpty) != 0
				&& (bProjectHasCode || (Result & ETargetPlatformReadyStatus::CodeBuildRequired)
					|| (!FApp::GetEngineIsPromotedBuild() && !FApp::IsEngineInstalled())))
			{
				AddMessageLog(
					LOCTEXT("RemoveServerNameNotFound", "Remote compiling requires a server name. "),
					CustomizedLogMessage.IsEmpty()
					? LOCTEXT("RemoveServerNameNotFoundDetail",
						"Please specify one in the Remote Server Name settings field.")
					: CustomizedLogMessage,
					NotInstalledTutorialLink,
					DocumentationLink
				);
				UnrecoverableError = true;
			}

			if ((Result & ETargetPlatformReadyStatus::CodeUnsupported) != 0)
			{
				FMessageDialog::Open(EAppMsgType::Ok,
					LOCTEXT("NotSupported_SelectedPlatform",
						"Sorry, packaging a code-based project for the selected platform is currently not supported. This feature may be available in a future release."));
				UnrecoverableError = true;
			}
			else if ((Result & ETargetPlatformReadyStatus::PluginsUnsupported) != 0)
			{
				FMessageDialog::Open(EAppMsgType::Ok,
					LOCTEXT("NotSupported_ThirdPartyPlugins",
						"Sorry, packaging a project with third-party plugins is currently not supported for the selected platform. This feature may be available in a future release."));
				UnrecoverableError = true;
			}

			if (UnrecoverableError)
			{
				PostPackageProject.ExecuteIfBound(false);
				return;
			}
		}
	}

#if ENGINE_MAJOR_VERSION >= 5
	FName VanillaPlatformName = PlatformInfo->Name;
	while (PlatformInfo != PlatformInfo->VanillaInfo)
	{
		PlatformInfo = PlatformInfo->VanillaInfo;
		VanillaPlatformName = PlatformInfo->Name;
	}
#else
	FName VanillaPlatformName = PlatformInfo->VanillaPlatformName;
#endif

	if (!FModuleManager::LoadModuleChecked<IProjectTargetPlatformEditorModule>("ProjectTargetPlatformEditor").
		ShowUnsupportedTargetWarning(VanillaPlatformName))
	{
		PostPackageProject.ExecuteIfBound(false);
		return;
	}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
	UPlatformsMenuSettings* PlatformsSettings = GetMutableDefault<UPlatformsMenuSettings>();
	FDirectoryPath& StagingDirectory = PlatformsSettings->StagingDirectory;
#else
	FDirectoryPath& StagingDirectory = PackagingSettings->StagingDirectory;
#endif

	// let the user pick a target directory
	if (StagingDirectory.Path.IsEmpty() || StagingDirectory.Path == FPaths::ProjectDir())
	{
		FString OutFolderName;

		void* ParentWindowWindowHandle = nullptr;
		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		if (!FDesktopPlatformModule::Get()->OpenDirectoryDialog(ParentWindowWindowHandle,
			LOCTEXT("PackageDirectoryDialogTitle",
				"Package project...")
			.ToString(),
			StagingDirectory.Path,
			OutFolderName))
		{
			PostPackageProject.ExecuteIfBound(false);
			return;
		}

		StagingDirectory.Path = OutFolderName;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
		PlatformsSettings->SaveConfig();
#else
		PackagingSettings->SaveConfig();
#endif

	}

	OutFolderPath = StagingDirectory.Path;

	// create the packager process
	FString OptionalParams;

	if (PackagingSettings->FullRebuild)
	{
		OptionalParams += TEXT(" -clean");
	}

	if (PackagingSettings->bCompressed)
	{
		OptionalParams += TEXT(" -compressed");
	}

	OptionalParams += GetCookingOptionalParams();

	if (PackagingSettings->bUseIoStore)
	{
		OptionalParams += TEXT(" -iostore");

		// Pak file(s) must be used when using container file(s)
		PackagingSettings->UsePakFile = true;
	}

	if (PackagingSettings->UsePakFile)
	{
		OptionalParams += TEXT(" -pak");
	}

	if (PackagingSettings->bUseIoStore)
	{
		OptionalParams += TEXT(" -iostore");
	}

	if (PackagingSettings->bMakeBinaryConfig)
	{
		OptionalParams += TEXT(" -makebinaryconfig");
	}

	if (PackagingSettings->IncludePrerequisites)
	{
		OptionalParams += TEXT(" -prereqs");
	}

	if (!PackagingSettings->ApplocalPrerequisitesDirectory.Path.IsEmpty())
	{
		OptionalParams += FString::Printf(
			TEXT(" -applocaldirectory=\"%s\""), *(PackagingSettings->ApplocalPrerequisitesDirectory.Path));
	}
	else if (PackagingSettings->IncludeAppLocalPrerequisites)
	{
		OptionalParams += TEXT(" -applocaldirectory=\"$(EngineDir)/Binaries/ThirdParty/AppLocalDependencies\"");
	}

	if (PackagingSettings->ForDistribution)
	{
		OptionalParams += TEXT(" -distribution");
	}

	if (!PackagingSettings->IncludeDebugFiles)
	{
		OptionalParams += TEXT(" -nodebuginfo");
	}

	if (PackagingSettings->bGenerateChunks)
	{
		OptionalParams += TEXT(" -manifests");
	}

#if ENGINE_MAJOR_VERSION >= 5
	bool bTargetPlatformCanUseCrashReporter = PlatformInfo->DataDrivenPlatformInfo->bCanUseCrashReporter;
#else
	bool bTargetPlatformCanUseCrashReporter = PlatformInfo->bTargetPlatformCanUseCrashReporter;
#endif
	if (bTargetPlatformCanUseCrashReporter && TargetPlatformName == FName("WindowsNoEditor") &&
		PlatformInfo->PlatformFlavor == TEXT("Win32"))
	{
		FString MinumumSupportedWindowsOS;
		GConfig->GetString(
			TEXT("/Script/WindowsTargetPlatform.WindowsTargetSettings"), TEXT("MinimumOSVersion"),
			MinumumSupportedWindowsOS, GEngineIni);
		if (MinumumSupportedWindowsOS == TEXT("MSOS_XP"))
		{
			OptionalParams += TEXT(" -SpecifiedArchitecture=_xp");
			bTargetPlatformCanUseCrashReporter = false;
		}
	}

	// Append any extra UAT flags specified for this platform flavor
	if (!PlatformInfo->UATCommandLine.IsEmpty())
	{
		OptionalParams += TEXT(" ");
		OptionalParams += PlatformInfo->UATCommandLine;
	}
	else
	{
		// At the moment we only support Windows64
		OptionalParams += TEXT(" -platform=");
		OptionalParams += TEXT("Win64");
	}

	// Get the target to build
	const FTargetInfo* Target = PackagingSettings->GetBuildTargetInfo();

	// Only build if the user elects to do so
	bool bBuild = false;
	if (PackagingSettings->Build == EProjectPackagingBuild::Always)
	{
		bBuild = true;
	}
	else if (PackagingSettings->Build == EProjectPackagingBuild::Never)
	{
		bBuild = false;
	}
	else if (PackagingSettings->Build == EProjectPackagingBuild::IfProjectHasCode)
	{
		bBuild = true;
		if (FApp::GetEngineIsPromotedBuild() && !bAssetNativizationEnabled)
		{
			FString BaseDir;

			// Get the target name
			FString TargetName;
			if (Target == nullptr)
			{
				TargetName = TEXT("UE4Game");
			}
			else
			{
				TargetName = Target->Name;
			}

			// Get the directory containing the receipt for this target, depending on whether the project needs to be built or not
			FString ProjectDir = FPaths::GetPath(FPaths::GetProjectFilePath());
			if (Target != nullptr && FPaths::IsUnderDirectory(Target->Path, ProjectDir))
			{
				UE_LOG(LogInnoactivePortal, Log, TEXT("Selected target: %s"), *Target->Name);
				BaseDir = ProjectDir;
			}
			else
			{
				FText Reason;
				if (Platform->RequiresTempTarget(bProjectHasCode, ConfigurationInfo.Configuration, false, Reason))
				{
					UE_LOG(LogInnoactivePortal, Log, TEXT("Project requires temp target (%s)"), *Reason.ToString());
					BaseDir = ProjectDir;
				}
				else
				{
					UE_LOG(LogInnoactivePortal, Log, TEXT("Project does not require temp target"));
					BaseDir = FPaths::EngineDir();
				}
			}
		}
	}
	else if (PackagingSettings->Build == EProjectPackagingBuild::IfEditorWasBuiltLocally)
	{
		bBuild = !FApp::GetEngineIsPromotedBuild();
	}
	if (bBuild)
	{
		OptionalParams += TEXT(" -build");
	}

	// Whether to include the crash reporter.
	if (PackagingSettings->IncludeCrashReporter && bTargetPlatformCanUseCrashReporter)
	{
		OptionalParams += TEXT(" -CrashReporter");
	}

	if (PackagingSettings->bBuildHttpChunkInstallData)
	{
		OptionalParams += FString::Printf(
			TEXT(" -manifests -createchunkinstall -chunkinstalldirectory=\"%s\" -chunkinstallversion=%s"),
			*(PackagingSettings->HttpChunkInstallDataDirectory.Path),
			*(PackagingSettings->HttpChunkInstallDataVersion));
	}

#if ENGINE_MAJOR_VERSION < 5
	int32 NumCookers = GetDefault<UEditorExperimentalSettings>()->MultiProcessCooking;
	if (NumCookers > 0)
	{
		OptionalParams += FString::Printf(TEXT(" -NumCookersToSpawn=%d"), NumCookers);
	}
#endif

	if (Target == nullptr)
	{
		OptionalParams += FString::Printf(TEXT(" -clientconfig=%s"), LexToString(ConfigurationInfo.Configuration));
	}
	else if (Target->Type == EBuildTargetType::Server)
	{
		OptionalParams += FString::Printf(
			TEXT(" -target=%s -serverconfig=%s"), *Target->Name, LexToString(ConfigurationInfo.Configuration));
	}
	else
	{
		OptionalParams += FString::Printf(
			TEXT(" -target=%s -clientconfig=%s"), *Target->Name, LexToString(ConfigurationInfo.Configuration));
	}

	FString ProjectPath = FPaths::IsProjectFilePathSet()
		? FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath())
		: FPaths::RootDir() / FApp::GetProjectName() / FApp::GetProjectName() + TEXT(
			".uproject");
	FString CommandLine = FString::Printf(
		TEXT(
			"-ScriptsForProject=\"%s\" BuildCookRun %s%s -nop4 -project=\"%s\" -cook -stage -archive -archivedirectory=\"%s\" -package -ue4exe=\"%s\" %s -utf8output"),
		*ProjectPath,
		GetUATCompilationFlags(),
		FApp::IsEngineInstalled() ? TEXT(" -installed") : TEXT(""),
		*ProjectPath,
		*StagingDirectory.Path,
		*FUnrealEdMisc::Get().GetExecutableForCommandlets(),
		*OptionalParams
	);

	IUATHelperModule::Get().CreateUatTask(CommandLine, PlatformInfo->DisplayName,
		LOCTEXT("PackagingProjectTaskName", "Packaging project"),
		LOCTEXT("PackagingTaskName", "Packaging"),
#if ENGINE_MAJOR_VERSION >= 5
		FAppStyle::GetBrush(TEXT("MainFrame.PackageProject")), nullptr,
#else
		FEditorStyle::GetBrush(TEXT("MainFrame.PackageProject")),
#endif
		[PostPackageProject](FString Message, double TimeSec)
		{
			AsyncTask(ENamedThreads::GameThread, [Message, PostPackageProject]()
				{
					PostPackageProject.ExecuteIfBound(Message == TEXT("Completed"));
				});
		}
	);
}
