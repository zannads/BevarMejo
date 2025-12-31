import importlib.resources
try:
    __version__ = importlib.resources.files("pybeme").joinpath("VERSION").read_text().strip()
except Exception:
    __version__ = "unknown"
    
from .simulator import Simulator
from .experiment import Experiment, load_experiments
from ._config import set_beme_project_path, get_beme_project_path

__all__ = [
    'Simulator',
    'Experiment',
    'load_experiments',
    'set_beme_project_path',
    'get_beme_project_path'
]