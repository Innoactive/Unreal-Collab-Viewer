import configparser
import os
import re
import unreal
import zipfile
from pathlib import Path
from portal_client.applications import list_applications, list_application_versions
from portal_client.application_uploader import ApplicationUploader
from portal_client.defaults import get_portal_backend_endpoint
from portal_client.organizations import get_organization_details
from portal_client.users import current_user

def get_username():
    return current_user().get("full_name")

def get_applications(organization_id):
    app_response = list_applications(page_size='100000', organization=organization_id)
    return app_response.get("results")

def get_portal_application_array(organization_id):
    arr = []
    
    apps = get_applications(organization_id)
    for app in apps:
        if app["target_platform"] != "windows":
            continue
        
        new_app = unreal.PortalApplication()
        new_app.name = app["name"]
        new_app.id = app["id"]
        new_app.identity = app["identity"]
        new_app.version = app["version"]
        arr.append(new_app)

    current_identity = unreal.InnoactivePortalSettings.get_app_identity()
    doesExist = any(app["identity"] == current_identity for app in apps)

    if doesExist == False:
        new_app = unreal.PortalApplication()
        new_app.name = get_application_name()
        new_app.version = get_application_version()
        arr.insert(0, new_app)
    
    return arr

def get_current_application_index(apps):
    current_identity = unreal.InnoactivePortalSettings.get_app_identity()
    for index, app in enumerate(apps):
        if app.identity == current_identity:
            return index
    return 0

def fill_application_names(combobox, apps):
    for app in apps:
        combobox.add_option(app.name)

    app_index = get_current_application_index(apps)
    combobox.set_selected_index(app_index)

def get_archive_path(app_name, version):
    archive_folder = unreal.InnoactivePortalSettings.get_temp_archive_path()

    if not os.path.exists(archive_folder):
        os.makedirs(archive_folder)

    archive_name = os.path.join(archive_folder, f'{app_name}_{version}.zip')
    return archive_name

def archive_folder_with_progress(folder_path, zip_path):
    total_size = sum(file.stat().st_size for file in Path(folder_path).rglob('*'))
    text_label = "Archiving application..."

    with unreal.ScopedSlowTask(total_size, text_label) as slow_task:
        slow_task.make_dialog(True)

        with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            for root, _, files in os.walk(folder_path):
                for file in files:
                    if slow_task.should_cancel():
                        raise Exception("Archive operation cancelled by user")

                    file_path = os.path.join(root, file)
                    rel_path = os.path.relpath(file_path, folder_path)
                    zipf.write(file_path, rel_path)
                    slow_task.enter_progress_frame(os.path.getsize(file_path))

def upload_application(app_archive_path, version, changelog, organization_id, name="", identity=None):
    try:
        unreal.log("Uploading application...")

        config_parameters = {}
        config_parameters["version"] = version
        config_parameters["description_html"] = changelog
        config_parameters["organization_ids"] = [organization_id]
        config_parameters["identity"] = identity
        config_parameters["name"] = name
        config_parameters["current_version"] = True

        # If the application is a UE app, find the executable path to make sure the app is validated
        if (is_unreal_engine_app(app_archive_path)):
            executable_path = detect_ue_app_executable_path(app_archive_path)
            if executable_path:
                config_parameters["executable_path"] = executable_path
        
        total_frames = 1
        text_label = "Uploading application..."
        with unreal.ScopedSlowTask(total_frames, text_label) as slow_task:
            slow_task.make_dialog(True)

            uploader = ApplicationUploader(get_portal_backend_endpoint())
            response = uploader.upload_application(app_archive_path, config_parameters, slow_task)

        unreal.log("Uploaded application...")

        return response
    except Exception as e:
        unreal.log_warning("Failed to upload application...")
        unreal.log_warning(e)

def get_organization_domain(organization_id):
    organization = get_organization_details(organization_id)
    return organization.get("domain")

def application_config_parser():
    config_parser = configparser.ConfigParser(strict=False)
    config_parser.optionxform = str

    active_configuration_directory = unreal.Paths().project_config_dir()
    ini_file_path = f'{active_configuration_directory}/DefaultGame.ini'
    config_parser.read(ini_file_path, encoding="utf-8")

    return config_parser

def get_application_name():
    config_parser = application_config_parser()
    app_name = config_parser["/Script/EngineSettings.GeneralProjectSettings"].get("ProjectName")
    if app_name:
        return app_name
    else:
        return "Untitled Project"
    
def get_application_version():
    config_parser = application_config_parser()
    app_version = config_parser["/Script/EngineSettings.GeneralProjectSettings"].get("ProjectVersion")
    if app_version:
        return app_version
    else:
        return "1.0.0"
    
def calculate_next_version(version):
    try:
        version = version.split('.')
        version[1] = str(int(version[1]) + 1)
        return '.'.join(version)
    except:
        # Use project version if we cannot calculate next version
        return get_application_version()
    
def validate_version(version):
    if re.match(r'^(\d+)\.(\d+)\.(\d+)(?:\.(\d+))?$', version):
        return True
    return False

def get_application_versions(application_id):
    version_array = unreal.Array(str)

    if application_id == None or application_id == 0:
        return version_array 

    versions_response = list_application_versions(application_id)
    results = versions_response.get("results")

    for app_version in results:
        version_array.append(app_version["version"])

    return version_array

def detect_ue_app_executable_path(archive_path):
    invalid_executables = [
        "UE4PrereqSetup_x64.exe",
        "UEPrereqSetup_x64.exe",
    ]
    
    with zipfile.ZipFile(archive_path, "r", allowZip64=True) as zip_in:
        file_paths = zip_in.namelist()

    executables = [
        file_path
        for file_path in file_paths
        if (
            file_path.endswith(".exe")
            and not any(file_path.endswith(item) for item in invalid_executables)
        )
    ]

    # Find the executable that is in the root folder
    root_folder = os.path.commonpath(executables)
    
    matching_strings = [
        file_path
        for file_path in executables
        if os.path.dirname(file_path) == root_folder
    ]
    return matching_strings[0]


def is_unreal_engine_app(archive_path):
    with zipfile.ZipFile(archive_path, "r", allowZip64=True) as zip_in:
        file_paths = zip_in.namelist()

    if any(
        re.search(
            "(?:UE4PrereqSetup_x64.exe|UEPrereqSetup_x64.exe)$",
            file_path,
            re.IGNORECASE,
        )
        for file_path in file_paths
    ):
        return True
    
    return False