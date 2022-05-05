# AWS GameKit Plugin for Unreal

## Dev Setup

### Unreal/C++
**The plugin is developed using Unreal version 4.27. Install before proceeding.**

Game projects created in Unreal versions later than 4.27 are not backward compatible. AWS GameKit supports Unreal Engine version 4.27.

#### Build

##### Windows
1. Create the Visual Studio Solution for your game. In File Explorer, right click on `[your_game].uproject` and select "Generate Visual Studio project files"

2. Build [GameKit C++](https://github.com/aws/aws-gamekit) and copy over the DLL/PDB files. Make sure to checkout the matching version from the .gkcpp_version file in this repository. Note: this step is only needed if the plugin is being rebuilt. Prebuilt plugin for Windows, macOS, Android and iOS can be downloaded from this repository's [Releases](https://github.com/aws/aws-gamekit-unreal/releases).

##### macOS
1. Generate the Xcode workspace

```
/Users/Shared/Epic\ Games/UE_4.27/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh -project <path to your .uproject file>
```

2. Build [GameKit C++](https://github.com/aws/aws-gamekit) and copy over the libraries. Make sure to checkout the matching version from the .gkcpp_version file in this repository. Note: this step is only needed if the plugin is being rebuilt. Prebuilt plugin for Windows, macOS, Android and iOS can be downloaded from this repository's [Releases](https://github.com/aws/aws-gamekit-unreal/releases).

##### Android and iOS
Detailed steps for building and packaging a game for Android and iOS are available in the [Game Packaging section of our Production Readiness Guide](https://docs.aws.amazon.com/gamekit/latest/DevGuide/plugin-unreal-production.html).

### Using the Plugin
Check the updated [GUIDE](https://docs.aws.amazon.com/gamekit/latest/DevGuide/setting-up.html) for indepth details about how to use AWS GameKit.