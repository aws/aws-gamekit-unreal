# AWS GameKit Unreal Plugin

## Prerequisites
The plugin is developed using Unreal version 4.26. Make sure you have that installed before proceeding.

You also need to clone the AWS GameKit C++ [repository](https://github.com/aws/aws-gamekit).

## Dev Setup

1. Create an empty Unreal C++ project and copy the contents of this repo into your `<YOUR_SANDBOX_GAME>\Plugins` directory. Create the `Plugins\` directory if it doesn't exist yet.
2. Create the Visual Studio Solution for your game: in File Explorer, right click on `<YOUR_SANDBOX_GAME>.uproject` and select "Generate Visual Studio project files"
3. To copy over the DLL/PDB files from GameKit C++, see section `Copy Binaries to Plugin Libraries` in the [GameKit C++ README](https://github.com/aws/aws-gamekit/README.md).

### Lambda/Python
See Resources [README](https://github.com/aws/aws-gamekit-unreal/Resources/README.md).
