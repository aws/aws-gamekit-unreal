# AWS GameKit Plugin for Unreal

## Dev Setup

### Unreal/C++
**The plugin is developed using Unreal version 4.27. Install before proceeding.**

Game projects created in Unreal versions later than 4.27 are not backward compatible. AWS GameKit supports the latest stable version of Unreal Engine which is currently 4.27.

#### Build:
1. Create the Visual Studio Solution for your game. In File Explorer, right click on `[your_game].uproject` and select "Generate Visual Studio project files"
2. Build [GameKit C++](https://github.com/aws/aws-gamekit) and copy over the DLL/PDB files. Make sure to checkout the matching version from the .gkcpp_version file in this repository.

### Using the Plugin
Check the updated [GUIDE](https://docs.aws.amazon.com/gamekit/latest/DevGuide/setting-up.html) for indepth details about how to use AWS GameKit.
