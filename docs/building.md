# Installation Guide for Bèvar Méjo

This guide will walk you through downloading, building, and running the Bèvar Méjo project.

## What You'll Need

This project contains both C++ code (which needs to be compiled) and Python code working with Python 3.12 (which will need a dedicated environment). Don't worry if you're unfamiliar with C++ or Python - we'll guide you through each step!

## Supported Platforms and Development Environments

We support the following development environments:

- **Windows** with Visual Studio 2022 (VSCode should also work)
- **macOS** with Visual Studio Code
- **Linux Ubuntu** with command line tools

**Note**: Currently, only macOS with Visual Studio Code has been fully tested and works 100%. All other platforms are expected to work following the platform-specific instructions, but may require additional troubleshooting.

## Part A: C++ Installation

### Step 1: Install Required Tools

Choose the section that matches your operating system:

#### Windows Users

- Download and install [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) (Community edition is free)
- Make sure to include the "Desktop development with C++" workload during installation

#### macOS Users

- Install [Visual Studio Code](https://code.visualstudio.com/)
- Install the developer toolkit by running in Terminal: `xcode-select --install`

#### Linux Ubuntu Users

Open Terminal and run:

```bash
sudo apt update
sudo apt install git g++ cmake
```

### Step 2: Set Up Workspace and Dependencies

**Important**: We expect all repositories necessary to build this project to be in the same folder. Here's the expected structure:

```
/ (workspace folder)
├── BevarMejo/
├── vcpkg/
├── EPANET/
└── (other dependencies as needed)
```

Navigate to your chosen workspace folder in your terminal/command prompt:

```bash
cd /your/path/to/workspace_folder
```

**Note**: You can install dependent libraries in a custom path (e.g., `/opt`), but you'll need to specify absolute paths using CMake flags. See the ["Custom Installation Flags"](#custom-installation-flags) section for details.

#### Step 2.1: Install Vcpkg (Library Manager)

Most libraries are installed using Vcpkg, a C++ package manager that simplifies library installation.

For detailed instructions, visit the [official Vcpkg documentation](https://learn.microsoft.com/en-us/vcpkg/). Here's the quick setup:

**For Windows**:

```cmd
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

**For macOS/Linux**:

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
```

#### Step 2.2: Install Pagmo2

Pagmo2 provides the optimization algorithms used in this project.

Install via Vcpkg (recommended):

```bash
./vcpkg install pagmo2
```

**Note for Windows users**: Use `.\vcpkg` instead of `./vcpkg`

For alternative installation methods, see the [Pagmo2 documentation](https://esa.github.io/pagmo2/).

#### Step 2.3: Install JSON Library

We use the JSON for Modern C++ library by NLohmann for JSON processing.

Install via Vcpkg:

```bash
./vcpkg install nlohmann-json
```

Alternatively, you can install it using CMake in your workspace folder. See the [library documentation](https://json.nlohmann.me) for details.

#### Step 2.4: Install EPANET

EPANET (OWA-EPANET) is used to model water distribution systems and solve hydraulic equations. This library is not available through Vcpkg and must be installed manually. 

```bash
cd ..  # Go back to workspace folder
git clone https://github.com/OpenWaterAnalytics/EPANET.git
```

To build EPANET, you can follow the building instructions in the [EPANET BUILDING.md file](https://github.com/OpenWaterAnalytics/EPANET/blob/dev/BUILDING.md), but it is basically:

```bash
cd EPANET
mkdir build
git checkout origin/dev
cmake -DCMAKE_BUILD_TYPE:STRING=Release -S . -B build # On Windows, add -A x64 for a 64-bit build when MS Visual Studio is the compiler.
cmake --build build --config Release
```

### Step 3: Build Bèvar Méjo

Download the main project in your workspace folder, navigate to the project folder and create build directories:

```bash
git clone https://github.com/zannads/BevarMejo.git
cd BevarMejo
mkdir -p builds/releases/latest
```

**Note:** The build system will find it automatically since EPANET is in the workspace folder. However, if you installed Vcpkg outside the default path (`/opt/vcpkg/`), you need to specify its location:

```bash
# Generate build system
cmake -DCMAKE_BUILD_TYPE:STRING=Release \
      -DCMAKE_TOOLCHAIN_FILE=<path/to/vcpkg>/scripts/buildsystems/vcpkg.cmake \
      -S. -Bbuilds/releases/latest

# Build the project
cmake --build builds/releases/latest --config Release
```

#### Custom Installation Flags

You can customise the build process with these CMake flags:

- **Custom JSON library path**: If you installed JSON via CMake instead of Vcpkg or used a different name:
    
    ```
    -DJSON_ROOT:PATH=/your/path
    ```
    
- **Custom EPANET library path**: If you installed EPANET outside the workspace or with a different name:
    
    ```
    -DEPANET_ROOT:PATH=/your/path
    ```
    
    _Note_: Build files are expected in the `build` subfolder, unless you are trying to reproduce old results or build an older version of this software (see below the "Set project version flag").
    
- **Change output JSON style**: Customize the JSON key format:
    
    ```
    -DOUT_STYLE:STRING="option"
    ```
    
    Options: DEFAULT: `"snake_case"`, `"Sentence case"`, `"CamelCase"`, `"kebab-case"`, `"pascalCase"`.
    
- **Set project version**: Change the project version (default: `"latest"`):
    
    ```
    -DPROJECT_VERSION:STRING=version_name
    ```
    
    _Note_: This requires additional setup. See ["Installation for Results Reproduction"](#installation-for-results-reproduction) section.
    *Another note*: You need to specify the version with the `yy.m.d` format.

## Part B: Python Installation

### Step 1: Create Python Environment

Create and activate a Python 3.12 environment:

```bash
python3.12 -m venv beme
source beme/bin/activate  # On Windows: beme\Scripts\activate
```

### Step 2: Install Python Dependencies

Install basic required packages:

```bash
pip install pandas numpy epyt
```

### Step 3: Install PyGMO (Optional but Recommended)

**Important**: The PyGMO pip package is not well-maintained. For Python 3.12, you'll need to build from source.

**Recommended approach**:

1. Install Pagmo2 and pybind11 using Vcpkg (if not already done)
2. Build PyGMO from source using CMake. Set `PYGMO_INSTALL_PATH` to your Python environment's site-packages directory.

For detailed instructions, see the [PyGMO installation guide](https://esa.github.io/pygmo2/install.html#installation-from-source).

## Installation for Results Reproduction

During development, some results were published before certain bugs were discovered and fixed. To maintain backwards compatibility and ensure reproducibility of published work, I implemented a system of C preprocessor flags that preserves both the original (buggy) and corrected versions of the code.

Therefore, if you're reproducing results from published work, you must use specific versions of both the BeMe and EPANET projects that correspond to the version used when those results were generated. See ["About Versioning"](versioning.md).

During the part A installation, modify steps 2.4 and 3 as follows:

1. **Create the version-specific builds** for EPANET:

```bash
cd path/to/EPANET # Navigate to the folder and add a second remote (my fork)
git remote add zannads https://github.com/zannads/EPANET.git 

# Create the correct file hierarchy for the BevarMejo project
mkdir beme-releases
cd beme-releases/
mkdir 24.6.18 24.12.21

# Check out the first snapshot and build that version
git checkout zannads/beme-dev-v240618
cmake -DCMAKE_BUILD_TYPE:STRING=Release -S .. -B 24.6.18/ 
cmake --build 24.6.18/ --config Release

# Same for the second snapshot
git checkout zannads/beme-dev-v241221
cmake -DCMAKE_BUILD_TYPE:STRING=Release -S .. -B 24.12.21/
cmake --build 24.12.21/ --config Release
```

2. **Create a version-specific build** for the BeMe:

```bash
# 1. Navigate to the BeMe folder 
cd path/to/BevarMejo

# 2. Create the build tree: 
mkdir -p builds/releases/your_version

# 3. Build with version flag
cmake -DCMAKE_BUILD_TYPE:STRING=Release \
	  -DCMAKE_TOOLCHAIN_FILE=<path/to/vcpkg>/scripts/buildsystems/vcpkg.cmake \
	  -DPROJECT_VERSION:STRING=your_version \
	  -S. -Bbuilds/releases/your_version
    
cmake --build builds/releases/your_version --config Release
```

The following versions are required for full results reproduction: `v24.04.00`, `v24.06.00`, `v24.10.00`, `v24.11.00`, `v24.12.00`, and `latest`.

## Troubleshooting

### Common Issues

- **CMake not found**: Ensure CMake is installed. On Windows, it comes with Visual Studio.
- **Library not found**: Verify all libraries are in the correct locations and the Vcpkg installation completed successfully.
- **Permission errors (macOS/Linux)**: You may need `sudo` for some installation commands.
- **Python version issues**: Ensure you're using Python 3.12 for best compatibility.

## What's Next?

Once installation is complete, you'll be ready to run optimisation problems! Check out the usage guide for examples of how to use the software.

---

_This installation guide is designed to be beginner-friendly. If you have suggestions for improvements, please let us know!_