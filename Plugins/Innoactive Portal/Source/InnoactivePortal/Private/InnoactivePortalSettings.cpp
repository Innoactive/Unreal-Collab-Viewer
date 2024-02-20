// Copyright Epic Games, Inc. All Rights Reserved.

#include "InnoactivePortalSettings.h"
#include "DesktopPlatformModule.h"

UInnoactivePortalSettings::UInnoactivePortalSettings(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
        , BackendURL("https://api.innoactive.io")
        , ClientId("mrqJ6JDyrL2UDmXLMacia0uAaKDcsgFoQ8F3TCLZ")
        , RedirectPort(50978)
        , AppIdentity()
        , AccessToken()
        , RefreshToken()
        , OrganizationId(0)
        , TempArchivePath( { FDesktopPlatformModule::Get()->GetUserTempPath() })
		, bDeleteTempArchive(true)
{
}

FString UInnoactivePortalSettings::GetAppIdentity()
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    return Settings->AppIdentity;
}

void UInnoactivePortalSettings::SetAppIdentity(FString identity)
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    Settings->AppIdentity = identity;
    Settings->SaveConfig();
}

FString UInnoactivePortalSettings::GetAccessToken()
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    return Settings->AccessToken;
}

void UInnoactivePortalSettings::SetAccessToken(FString token)
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    Settings->AccessToken = token;
    Settings->SaveConfig();
}

FString UInnoactivePortalSettings::GetRefreshToken()
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    return Settings->RefreshToken;
}

void UInnoactivePortalSettings::SetRefreshToken(FString token)
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    Settings->RefreshToken = token;
    Settings->SaveConfig();
}

int32 UInnoactivePortalSettings::GetOrganizationId()
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    return Settings->OrganizationId;
}

void UInnoactivePortalSettings::SetOrganizationId(int32 orgId)
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    Settings->OrganizationId = orgId;
    Settings->SaveConfig();
}

FString UInnoactivePortalSettings::GetBackendURL()
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    return Settings->BackendURL;
}

FString UInnoactivePortalSettings::GetClientId()
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    return Settings->ClientId;
}

int32 UInnoactivePortalSettings::GetRedirectPort()
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    return Settings->RedirectPort;
}

FString UInnoactivePortalSettings::GetTempArchivePath()
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

    FString tempArchivePath = Settings->TempArchivePath.Path;
    return tempArchivePath;
}

bool UInnoactivePortalSettings::GetDeleteTempArchive()
{
    UInnoactivePortalSettings* Settings = GetMutableDefault<UInnoactivePortalSettings>();

	return Settings->bDeleteTempArchive;
}