// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "InnoactivePortalSettings.generated.h"

/**
* Implements the settings for the InnoactivePortal plugin.
*/
UCLASS(Config = EditorPerProjectUserSettings, DefaultConfig)
class INNOACTIVEPORTAL_API UInnoactivePortalSettings : public UObject
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, config, Category = "Innoactive Portal")
	FString BackendURL;

	UPROPERTY(EditAnywhere, config, Category = "Innoactive Portal")
	FString ClientId;

	UPROPERTY(EditAnywhere, config, Category = "Innoactive Portal")
	int32 RedirectPort;

	UPROPERTY(EditAnywhere, config, Category = "Innoactive Portal")
	FString AppIdentity;

	UPROPERTY(EditAnywhere, config, Category = "Authorization")
	FString AccessToken;

	UPROPERTY(EditAnywhere, config, Category = "Authorization")
	FString RefreshToken;

	UPROPERTY(EditAnywhere, config, Category = "Authorization")
	int32 OrganizationId;

	UPROPERTY(EditAnywhere, config, Category = Archiving)
	FDirectoryPath TempArchivePath;

	UPROPERTY(EditAnywhere, config, Category = Archiving)
	bool bDeleteTempArchive;

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static FString GetBackendURL();

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static FString GetClientId();

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static int32 GetRedirectPort();

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static void SetAppIdentity(FString identity);

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static FString GetAppIdentity();

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static void SetAccessToken(FString token);

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static FString GetAccessToken();

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static void SetRefreshToken(FString token);

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static FString GetRefreshToken();

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static void SetOrganizationId(int32 orgId);

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static int32 GetOrganizationId();

	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static FString GetTempArchivePath();
	
	UFUNCTION(BlueprintCallable, Category = "Innoactive Portal Settings")
	static bool GetDeleteTempArchive();
};