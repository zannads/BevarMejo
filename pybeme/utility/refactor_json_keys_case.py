import json
import sys
import os
from typing import Any, Dict, Callable, Union

# For now, only these are implemented...
def sentence_case_to_snake_case(text: str) -> str:
    """
    Convert a sentence case string to snake case.
    
    Args:
        text (str): The input string in sentence case format
                   (e.g., "Hello World Example")
    
    Returns:
        str: The string converted to snake case format
             (e.g., "hello_world_example")
    
    Raises:
        TypeError: If the input is not a string
    """
    if not isinstance(text, str):
        raise TypeError(f"Expected string input, got {type(text)}")
    
    return text.strip().replace(' ', '_').lower()


def kebab_case_to_snake_case(text: str) -> str:
    """
    Convert a kebab case string to snake case.
    
    Args:
        text (str): The input string in kebab case format
                   (e.g., "hello-world-example")
    
    Returns:
        str: The string converted to snake case format
             (e.g., "hello_world_example")
    
    Raises:
        TypeError: If the input is not a string
    """
    if not isinstance(text, str):
        raise TypeError(f"Expected string input, got {type(text)}")
    
    return text.strip().replace('-', '_')

def recursive_refactor(data: Any, to_case: Callable[[str], str]) -> Union[dict, list, Any]:
    """
    Recursively refactor the keys of a JSON-like object to the desired case.
    
    Args:
        data (Any): The input data structure to refactor. Can be a dictionary,
                   list, or primitive type.
        to_case (Callable[[str], str]): A function that takes a string and returns
                                       the transformed string in the desired case.
    
    Returns:
        Union[dict, list, Any]: The refactored data structure with transformed keys.
                               Maintains the same structure as the input but with
                               transformed dictionary keys.
    
    Examples:
        >>> data = {"firstName": {"innerKey": "value"}, "last_name": ["item1"]}
        >>> recursive_refactor(data, str.lower)
        {'firstname': {'innerkey': 'value'}, 'last_name': ['item1']}
    """
    if isinstance(data, dict):
        return {
            to_case(key): recursive_refactor(value, to_case)
            for key, value in data.items()
        }
    elif isinstance(data, list):
        return [recursive_refactor(item, to_case) for item in data]
    return data

def refactor_json_file(
    filepath: str, 
    from_case: str = 'Sentence case', 
    to_case: str = 'snake_case'
) -> str:
    """
    Refactor JSON file keys from one case format to another.
    
    Args:
        filepath (str): Path to the input JSON file
        from_case (str, optional): Input case format. Defaults to 'Sentence case'
        to_case (str, optional): Output case format. Defaults to 'snake_case'
    
    Returns:
        str: Path to the output JSON file
        
    Raises:
        ValueError: If case formats are not supported
        FileNotFoundError: If input file doesn't exist
        json.JSONDecodeError: If input file is not valid JSON
    """
    # Case format mappings
    CASE_MAPPINGS: Dict[str, str] = {
        'Sentence case': 'Sc',
        'kebab-case': 'kc',
        'snake_case': 'sc',
        'camelCase': 'cC',
        'PascalCase': 'PC'
    }
    
    # Supported case formats
    ALLOWED_INPUT_CASES = ['Sentence case', 'kebab-case']
    ALLOWED_OUTPUT_CASES = ['snake_case']
    
    # Validate input case format
    if from_case not in ALLOWED_INPUT_CASES:
        raise ValueError(
            f"Invalid input case format. Supported formats are: {ALLOWED_INPUT_CASES}"
        )
    
    # Validate output case format
    if to_case not in ALLOWED_OUTPUT_CASES:
        raise ValueError(
            f"Invalid output case format. Supported formats are: {ALLOWED_OUTPUT_CASES}"
        )
    
    # Select appropriate refactor function
    refactor_func: Callable[[str], str]
    if from_case == 'Sentence case':
        refactor_func = sentence_case_to_snake_case
    elif from_case == 'kebab-case':
        refactor_func = kebab_case_to_snake_case
    
    # Read and process JSON file
    with open(filepath, 'r') as f:
        data = json.load(f)
    
    # Refactor the data
    refactored_data = recursive_refactor(data, refactor_func)
    
    # Generate output filepath
    new_filepath = os.path.splitext(filepath)[0] + f'-{CASE_MAPPINGS[to_case]}.json'
    
    # Write refactored data
    with open(new_filepath, 'w') as f:
        json.dump(refactored_data, f, indent=2)
    
    return new_filepath


def main() -> None:
    """
    Main script entry point for command line usage.
    
    Usage:
        python refactor_json_keys_case.py <path_to_json_file> <from_case> <to_case>
    """
    # Check for minimum required arguments
    if len(sys.argv) < 2:
        print("Usage: python refactor_json_keys_case.py <path_to_json_file> "
              "<from_case> <to_case>")
        sys.exit(1)
    
    # Parse command line arguments
    filepath = sys.argv[1]
    from_case = sys.argv[2] if len(sys.argv) > 2 else 'Sentence case'
    to_case = sys.argv[3] if len(sys.argv) > 3 else 'snake_case'
    
    try:
        print(f"Refactoring keys in {filepath} from {from_case} to {to_case}...")
        new_filepath = refactor_json_file(filepath, from_case, to_case)
        print(f"Refactored data saved to {new_filepath}")
    
    except FileNotFoundError:
        print(f"Error: File '{filepath}' not found")
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Error: '{filepath}' is not a valid JSON file")
        sys.exit(1)
    except ValueError as e:
        print(f"Error: {str(e)}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {str(e)}")
        sys.exit(1)

if __name__ == '__main__':
    main()