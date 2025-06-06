# Installation Guide for Bèvar Méjo

This guide will walk you through downloading, building, and running the Bèvar Méjo project.

## What You'll Need

This project contains both C++ code (which needs to be compiled) and Python code working with Python 3.12 (which will need a dedicated environment). Don't worry if you're not familiar with C++ or Python - we'll guide you through each step!

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
- Make sure to include "Desktop development with C++" workload during installation

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

**Important**: We expect that all repositories necessary to build this project are located in the same folder. Here's the expected structure:

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

EPANET (OWA-EPANET) is used for modeling Water Distribution Systems and solving hydraulic equations. This library is not available through Vcpkg and must be installed manually.

Navigate back to your workspace folder:

```bash
cd ..  # Go back to workspace folder
git clone https://github.com/OpenWaterAnalytics/EPANET.git
```

Follow the building instructions in the [EPANET BUILDING.md file](https://github.com/OpenWaterAnalytics/EPANET/blob/dev/BUILDING.md).

### Step 3: Build Bèvar Méjo

Download the main project in your workspace folder, navigate to the project folder and create build directories:

```bash
git clone https://github.com/zannads/BevarMejo.git
cd BevarMejo
mkdir -p builds/releases/latest
```

**Note:** Since EPANET is in the workspace folder, the build system will find it automatically. However, if you installed Vcpkg outside the default path (`/opt/vcpkg/`), you need to specify its location:

```bash
# Generate build system
cmake -DCMAKE_BUILD_TYPE:STRING=Release \
      -DCMAKE_TOOLCHAIN_FILE=<path/to/vcpkg>/scripts/buildsystems/vcpkg.cmake \
      -S. -Bbuilds/releases/latest

# Build the project
cmake --build builds/releases/latest --config Release
```

#### Custom Installation Flags

You can customize the build process with these CMake flags:

- **Custom JSON library path**: If you installed JSON via CMake instead of Vcpkg or used a different name:
    
    ```
    -DJSON_ROOT:PATH=/your/path
    ```
    
- **Custom EPANET library path**: If you installed EPANET outside the workspace or with a different name:
    
    ```
    -DEPANET_ROOT:PATH=/your/path
    ```
    
    _Note_: Build files are expected in the `build` subfolder.
    
- **Change output JSON style**: Customize the JSON key format (default: `"snake_case"`):
    
    ```
    -DOUT_STYLE:STRING="option"
    ```
    
    Options: `"Sentence case"`, `"CamelCase"`, `"kebab-case"`, `"pascalCase"`
    
- **Set project version**: Change the project version (default: `"latest"`):
    
    ```
    -DPROJECT_VERSION:STRING=version_name
    ```
    
    _Note_: This requires additional setup. See ["Installation for Results Reproduction"](#installation-for-results-reproduction) section.
    
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

Therefore, if you're reproducing results from published work, you will need to use specific versions of both the BeMe and EPANET projects that correspond to the version used when those results were generated. See ["About Versioning"](versioning.md).

During the part A installation, modify steps 2.4 and 3 as follows:

1. **Create a version-specific build** for EPANET:
```bash
# 1. Navigate to the EPANET folder
cd path/to/EPANET

# 2. Create the build folder
	# option a: for versions before v25.02.00
# mkdir -p builds/24.6.18
	# option b: for versions starting from v25.02.00
mkdir build

# 3. Checkout the commit from the date using its hash value
	# option a: for versions before v25.02.00: June 18, 2024
# git checkout -b c24-06-18 <sha1>
	# option b: for versions starting from  v25.02.00: December 21, 2024
# git checkout -b c24-12-21 <sha1>

# 4. Build EPANET in that folder
cmake -DCMAKE_BUILD_TYPE:STRING=Release \
	-S. -B<build_folder>
cmake --build <build_folder> --config release
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

For full results reproduction, the following versions are required: `v24.04.00`, `v24.06.00`, `v24.10.00`, `v24.11.00`, `v24.12.00`, and `latest`.
## Troubleshooting

### Common Issues

- **CMake not found**: Ensure CMake is installed. On Windows, it comes with Visual Studio.
- **Library not found**: Verify all libraries are in the correct locations and Vcpkg installation completed successfully.
- **Permission errors (macOS/Linux)**: You may need `sudo` for some installation commands.
- **Python version issues**: Ensure you're using Python 3.12 for best compatibility.

## What's Next?

Once installation is complete, you'll be ready to run optimization problems! Check out the usage guide for examples of how to use the software.

---

_This installation guide is designed to be beginner-friendly. If you have suggestions for improvements, please let us know!_