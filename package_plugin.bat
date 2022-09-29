@ECHO OFF
if [%1]==[] goto usage

PUSHD %1
git checkout mainline
git pull

call scripts\Win64\regenerate_projects.bat Debug
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\msbuild.exe" INSTALL.vcxproj -t:Rebuild /p:Configuration="Debug" /p:Platform="x64" -maxcpucount

call scripts\Win64\regenerate_projects.bat Release
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\msbuild.exe" INSTALL.vcxproj -t:Rebuild /p:Configuration="Release" /p:Platform="x64" -maxcpucount

call scripts\Win64\copyheaders.bat %~dp0
POPD

git submodule init
git submodule update --remote --force

ECHO Copying Aws GameKit Libraries to Plugin directory...
rmdir /S /Q %~dp0AwsGameKit\Binaries
del /Q %~dp0AwsGameKit\Libraries\Win64\Debug\*.dll
del /Q %~dp0AwsGameKit\Libraries\Win64\Debug\*.pdb
del /Q %~dp0AwsGameKit\Libraries\Win64\Release\*.dll
del /Q %~dp0AwsGameKit\Libraries\Win64\Release\*.pdb

mkdir %~dp0AwsGameKit\Libraries\Win64\Debug\
copy %1\install\Debug\bin\*.dll %~dp0AwsGameKit\Libraries\Win64\Debug\
copy %1\install\Debug\bin\*.pdb %~dp0AwsGameKit\Libraries\Win64\Debug\
mkdir %~dp0AwsGameKit\Libraries\Win64\Release\
copy %1\install\Release\bin\*.dll %~dp0AwsGameKit\Libraries\Win64\Release\
copy %1\install\Release\bin\*.pdb %~dp0AwsGameKit\Libraries\Win64\Release\

ECHO Generating Documentation
rmdir /Q /S %~dp0AwsGameKit\Docs
mkdir %~dp0AwsGameKit\Docs
PUSHD %1
doxygen Doxyfile-exports
move exports_docs %~dp0AwsGameKit\Docs
POPD
move %~dp0AwsGameKit\Docs\exports_docs %~dp0AwsGameKit\Docs\gamekitcpp
doxygen %~dp0AwsGameKit\Doxyfile

ECHO Packaging Plugin
call "C:\Program Files\Epic Games\UE_4.27\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="%~dp0AwsGameKit\AwsGameKit.uplugin" -Package=%~dp0plugin_package\AwsGameKit -Rocket -VS2019

cd plugin_package

ECHO Copying headers to final package
xcopy /E /I /Y /Q %~dp0AwsGameKit\Libraries\include AwsGameKit\Libraries\include

ECHO Copying Docs to final package
xcopy /E /I /Y /Q %~dp0AwsGameKit\Docs AwsGameKit\Docs

ECHO Copying Config/FilterPlugin.ini to final package
mkdir AwsGameKit\Config
copy /Y %~dp0AwsGameKit\Config\FilterPlugin.ini AwsGameKit\Config\FilterPlugin.ini

ECHO Compressing package
tar -acf AwsGameKit.zip AwsGameKit

ECHO Deleting temp files
rmdir /S /Q AwsGameKit
cd ..

goto done

:usage
ECHO Usage: package_plugin.bat ^<PATH_TO_GAMEKIT^>
ECHO ---
ECHO Example:
ECHO package_plugin.bat D:\workspace\GameKit\src\aws-gamekit
goto end

:done
ECHO Done.

:end
