import os
import unreal
from util import get_username
from auth import OAuth2AuthorizationCodeWithOrganization
from threading import Thread
from urllib.parse import urljoin

def get_oauth2_authorization_code():
    backend_url = unreal.InnoactivePortalSettings.get_backend_url()
    auth_url = urljoin(backend_url, "/oauth/authorize/from-unreal-engine/")
    token_url = urljoin(backend_url, "/oauth/token/")

    client_id = unreal.InnoactivePortalSettings.get_client_id()
    redirect_port = unreal.InnoactivePortalSettings.get_redirect_port()

    return OAuth2AuthorizationCodeWithOrganization(auth_url, token_url, client_id=client_id, scopes="read", redirect_uri_port=redirect_port, timeout=600)

def authorize_task(on_authentication_success):
    auth_code = get_oauth2_authorization_code()
    state, access_token, organization_id, expires_in, refresh_token = auth_code.request_new_token()
    unreal.InnoactivePortalBlueprintClass.publish_authentication_event(on_authentication_success, access_token, refresh_token, int(organization_id))

def validate_current_token(access_token, refresh_token, organization_id, on_authentication_success):
    # If access_token is empty or none, it means there is nothing to validate
    if access_token == None or access_token == "":
        return

    os.environ["PORTAL_BACKEND_ACCESS_TOKEN"] = access_token
    os.environ["PORTAL_BACKEND_ENDPOINT"] = unreal.InnoactivePortalSettings.get_backend_url()

    try:
        # If this works, it means we are already logged in
        get_username()
        unreal.InnoactivePortalBlueprintClass.publish_authentication_event(on_authentication_success, access_token, refresh_token, organization_id)
    except:
        try:
            # If getting the username fails, it means we need to refresh the token
            auth_code = get_oauth2_authorization_code()
            state, access_token, expires_in, refresh_token = auth_code.refresh_token(refresh_token)
            unreal.InnoactivePortalBlueprintClass.publish_authentication_event(on_authentication_success, access_token, refresh_token, organization_id)
        except:
            # If refreshing also fails, there is nothing to do
            return

@unreal.uclass()
class PythonBridgeImplementation(unreal.InnoactivePortalPythonBridge):
    @unreal.ufunction(override = True)
    def authenticate(self, OnAuthenticationSuccess):
        thread = Thread(target=authorize_task, args=(OnAuthenticationSuccess,))
        thread.start()

    @unreal.ufunction(override = True)
    def validate_current_token(self, access_token, refresh_token, organization_id, on_authentication_success):
        thread = Thread(target=validate_current_token, args=(access_token, refresh_token, organization_id, on_authentication_success))
        thread.start()