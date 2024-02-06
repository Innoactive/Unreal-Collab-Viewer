import json
from argparse import ArgumentParser
from urllib.parse import urljoin

import requests

from portal_client.defaults import get_portal_backend_endpoint
from portal_client.pagination import pagination_parser
from portal_client.utils import get_authorization_header


def list_organizations(**filters):
    organizations_url = urljoin(get_portal_backend_endpoint(), "/api/organizations/")
    response = requests.get(
        organizations_url,
        headers={"Authorization": get_authorization_header()},
        params=filters,
        timeout=5,
    )

    if not response.ok:
        print(response.json())
    response.raise_for_status()

    return response.json()


def get_organization_details(organization_id):
    organizations_url = urljoin(urljoin(get_portal_backend_endpoint(), "/api/organizations/"), str(organization_id))

    response = requests.get(
        organizations_url,
        headers={"Authorization": get_authorization_header()},
        timeout=5,
    )

    if not response.ok:
        print(response.json())
    response.raise_for_status()

    return response.json()


def list_organizations_cli(args):
    organizations_response = list_organizations(
        page=args.page,
        page_size=args.page_size,
    )

    print(json.dumps(organizations_response))


def configure_organizations_parser(parser: ArgumentParser):
    organization_parser = parser.add_subparsers(
        description="List and manage organizations on Portal"
    )

    organizations_list_parser = organization_parser.add_parser(
        "list",
        help="Returns a paginated list of organizations on Portal",
        parents=[pagination_parser],
    )

    organizations_list_parser.set_defaults(func=list_organizations_cli)
    return organization_parser
