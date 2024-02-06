#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InnoactivePortalBlueprint.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FPostAuthentication, FString, AccessToken, FString, RefreshToken, int, OrganizationId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPostPackageProject, bool, Success);

UCLASS()
class INNOACTIVEPORTAL_API UInnoactivePortalBlueprintClass : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Innoactive")
		static bool IsInitialized();

	UFUNCTION(BlueprintCallable, Category = "Innoactive")
		static void Authenticate(const FPostAuthentication& PostAuthentication);

	UFUNCTION(BlueprintCallable, Category = "Innoactive")
		static void ValidateCurrentToken(const FString& AccessToken, const FString& RefreshToken, const int32 OrganizationId, const FPostAuthentication& PostAuthentication);

	UFUNCTION(BlueprintCallable, Category = "Innoactive")
		static void PublishAuthenticationEvent(const FPostAuthentication& PostAuthentication, const FString AccessToken, const FString RefreshToken, const int32 OrganizationId);

	UFUNCTION(BlueprintCallable, Category = "Innoactive")
		static void OpenDirectoryDialog(const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName);

	UFUNCTION(BlueprintCallable, Category = "Innoactive")
		static void PackageProject(const FName InPlatformInfoName, const FPostPackageProject& PostPackageProject, FString& OutFolderName);
};