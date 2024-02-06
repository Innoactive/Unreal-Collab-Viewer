import requests
from requests_auth import oauth2_authentication_responses_server, OAuth2AuthorizationCode
from requests_auth.errors import InvalidGrantRequest, GrantNotProvided

def request_new_grant_with_post(url: str, data, grant_name: str, timeout: float, session: requests.Session) -> (str, int, str):
        with session:
            response = session.post(url, data=data, timeout=timeout)
            if not response:
                # As described in https://tools.ietf.org/html/rfc6749#section-5.2
                raise InvalidGrantRequest(response)

            content = response.json()
            organization_id = response.headers.get("portal-organization-id")
        token = content.get(grant_name)
        if not token:
            raise GrantNotProvided(grant_name, content)
        return token, organization_id, content.get("expires_in"), content.get("refresh_token")

class OAuth2AuthorizationCodeWithOrganization(OAuth2AuthorizationCode):
    def request_new_token(self):
        # Request code
        state, code = oauth2_authentication_responses_server.request_new_grant(
            self.code_grant_details
        )

        # As described in https://tools.ietf.org/html/rfc6749#section-4.1.3
        self.token_data["code"] = code
        # As described in https://tools.ietf.org/html/rfc6749#section-4.1.4
        token, organization_id, expires_in, refresh_token = request_new_grant_with_post(
            self.token_url,
            self.token_data,
            self.token_field_name,
            self.timeout,
            self.session,
        )
        # Handle both Access and Bearer tokens
        return (
            (self.state, token, organization_id, expires_in, refresh_token)
            if expires_in
            else (self.state, token, organization_id)
        )