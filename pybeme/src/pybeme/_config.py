import os
import pathlib
import warnings

# Internal state
_BEME_PROJ__PATH = None

def set_beme_project_path(path: str):
    """
    Explicitly set the path to the BevarMejo project (monorepo git folder of the project).
    This overrides environment variables and automatic discovery.
    """
    global _BEME_PROJ__PATH

    # Warn if set already: "this is a configuration function menat to be called onece after init" 
    if _BEME_PROJ__PATH is not None:
        warnings.warn(f"You are explicitly setting the path to the BevarMejo project again. Current path: {_BEME_PROJ__PATH}")

    absolute_path = pathlib.Path(path).resolve()
    
    if not absolute_path.exists():
        raise RuntimeError(
                "Impossible to set the path to the beme project",
                "Path doesn't exist",
                f"Path: '{absolute_path}'"
            )
    
    # Additional checks on the release folder.
    releases_dir = absolute_path / "releases"
    if not releases_dir.exists():
        raise RuntimeError(
                "Impossible to set the path to the beme project",
                "Path doesn't contain the 'releases' folder",
                f"Path: '{absolute_path}'"
            )
    
    # Get the name of the releases (directory in releases)
    releases = [item  for item in releases_dir.iterdir() if item.is_dir()]
    
    _BEME_PROJ__PATH = absolute_path
    print(f"Beme project path set to: {_BEME_PROJ__PATH}")
    print(f"Currently contains {len(releases)} releases")

def get_beme_project_path():
    # 1. Manual override (Highest priority)
    if _BEME_PROJ__PATH:
        return _BEME_PROJ__PATH

    # 2. Environment Variable
    env_path = os.getenv("BEME_PROJECT_DIR")
    if env_path:
        return pathlib.Path(env_path)

    # 3. Automatic discovery (Relative to library)
    fallback = pathlib.Path(__file__).parents[3]
    if fallback.exists():
        return fallback

    raise RuntimeError("Beme project directory not found. Use 'set_beme_project_path()' to point to it.")