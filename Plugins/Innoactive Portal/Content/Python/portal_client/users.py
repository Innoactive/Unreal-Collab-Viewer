import json
from argparse import ArgumentParser
from urllib.parse import urljoin

import requests

from portal_client.defaults import get_portal_backend_endpoint
from portal_client.organization import organization_parser
from portal_client.pagination import pagination_parser
from portal_client.utils import get_authorization_header

def current_user():
    current_user_url = urljoin(get_portal_backend_endpoint(), "/api/users/current")
    response = requests.get(
        current_user_url,
        headers={"Authorization": get_authorization_header()}
    )

    if not response.ok:
        print(response.json())
    response.raise_for_status()

    return response.json()

def list_users(**filters):
    users_url = urljoin(get_portal_backend_endpoint(), "/api/users/")
    response = requests.get(
        users_url,
        headers={"Authorization": get_authorization_header()},
        params=filters,
    )

    if not response.ok:
        print(response.json())
    response.raise_for_status()

    return response.json()


def list_users_cli(args):
    users_response = list_users(
        organization=args.organization,
        groups=args.user_groups,
        page=args.page,
        page_size=args.page_size,
        search=args.search,
    )

    print(json.dumps(users_response))


def create_user(**properties):
    users_url = urljoin(get_portal_backend_endpoint(), "/api/users/")
    response = requests.post(
        users_url,
        headers={"Authorization": get_authorization_header()},
        json=properties,
    )

    if not response.ok:
        print(response.json())
    response.raise_for_status()

    return response.json()


def create_user_cli(args):
    user_creation_response = create_user(
        email=args.email,
        organizations=args.organization_ids,
        first_name=args.first_name,
        last_name=args.last_name,
    )
    print(json.dumps(user_creation_response))


def configure_users_parser(parser: ArgumentParser):
    user_parser = parser.add_subparsers(
        description="List and manage user accounts on Portal"
    )

    users_list_parser = user_parser.add_parser(
        "list",
        help="Returns a paginated list of users on Portal",
        parents=[pagination_parser, organization_parser],
    )

    filters_group = users_list_parser.add_argument_group("filters", "Filtering Users")
    filters_group.add_argument(
        "--user-groups",
        metavar="GROUP_ID",
        type=int,
        default=[],
        nargs="+",
        help="Only return users within the given groups (ids)",
    )
    filters_group.add_argument(
        "--search",
        help="A search term (e.g. email address, user name) to filter results by",
    )

    users_list_parser.set_defaults(func=list_users_cli)

    user_create_parser = user_parser.add_parser(
        "create",
        help="Creates a new user account on Portal",
    )

    user_create_parser.add_argument("email", help="The user's e-mail address.")
    user_create_parser.add_argument(
        "--first-name", help="The user's first name.", required=True
    )
    user_create_parser.add_argument(
        "--last-name", help="The user's last name.", required=True
    )
    user_create_parser.add_argument(
        "--organization-ids",
        help="IDs of any organization the user should be part of.",
        default=[],
        nargs="+",
        required=True,
    )
    user_create_parser.set_defaults(func=create_user_cli)

    return user_parser
