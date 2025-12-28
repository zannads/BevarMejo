import pathlib
import subprocess

from packaging import version
V = version.Version

from .._config import get_beme_project_path

def get_bemelib_release_compat_versions(version_dir: pathlib.Path):
    try:
        # Run the command: beme-sim --compat-range
        result = subprocess.run(
            [str(version_dir)+"/cli/beme-sim", "--compat-range"],
            capture_output=True,
            text=True,    # Automatically decodes bytes to string
            check=True    # Raises an error if the C++ code crashes
        )
        
        # We expect the output to be "25.1.0-25.4.3", we parse it:
        output = result.stdout.strip()
        if "-" not in output:
            raise RuntimeError(
                f"Impossible to get the compatible versions for this beme executable: {version_dir}. "
                f"We expect a range of versions in output but we got: '{output}'"
            )

        start_v_str, end_v_str = output.split("-")
        start_v = version.parse(start_v_str)
        end_v = version.parse(end_v_str)
        
        return (start_v, end_v)
        
    except subprocess.CalledProcessError as e:
        raise RuntimeError(
            f"Impossible to get the compatible versions for this beme executable: {version_dir}. "
            "An error was raised while calling the beme executable.",
            f"Exit code: {e.returncode}\n"
            f"Stdout: {e.stdout}\n"
            f"Stderr: {e.stderr}"
        ) from e
    

def get_bemelib_installed_releases():
    """
    Returns a dictionary with the bemelib releases installed on the beme project path.
    """

    beme_dir = get_beme_project_path()
    releases_dir = beme_dir / "releases"
    if not releases_dir.exists():
        raise RuntimeError(
                f"Impossible to retrieve the bemelib releases in the beme project.",
                "Path doesn't contain the 'releases' folder",
                f"Path: '{beme_dir}'"
            )
    
    releases = [item for item in releases_dir.iterdir() if item.is_dir()]

    installed_releases: dict[str, tuple[V, V]] = {}
    for release in releases:
        try:
            installed_releases[str(release)] = get_bemelib_release_compat_versions(
                release
            )
        except Exception as e:
            print(
                f"Folder {release} in 'releases' directory does not contain a compatible bemelib release."
            )
            print(f"Exception: {e}")

    return installed_releases


def get_working_bemelib_release(problem_version: str):
    """
    Given a problem version (as int or string), returns the appropriate release version
    based on the version compatibility ranges.
    
    Args:
        problem_version: The version number of the problem file, either as int (e.g., 250200) 
                        or string (e.g., 'v25.02.0')
        
    Returns:
        str: The release version that can run this problem in format 'release/XX.XX.XX'
    """
    # Convert string version to integer if needed
    if isinstance(problem_version, str) and problem_version.startswith('v'):
        # Remove 'v' prefix and split by dots
        ver = version.parse(problem_version[1:])
        
    releases = get_bemelib_installed_releases()

    for release, (min_v, max_v) in releases.items():
        if min_v <= ver <= max_v:
            return release
        
    raise RuntimeError(
        "Impossible to find a working release of bemelib in the set project path."
    )

def get_beme_required_exact_en_version(problem_version:str) -> tuple:
    # Convert string version to integer if needed
    if isinstance(problem_version, str) and problem_version.startswith('v'):
        # Remove 'v' prefix and split by dots
        version_parts = problem_version[1:].split('.')
        
        # Convert to integer format (YYMMDD)
        major = int(version_parts[0]) * 10000
        minor = int(version_parts[1]) * 100
        patch = int(version_parts[2])
        int_version = major + minor + patch
    else:
        int_version = problem_version

    if int_version < 250200:
        return (24,6,18)
    else:
        return (24,12,21)
