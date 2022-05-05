#!/bin/bash
# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# This script is meant to update the AWS GameKit Plugin zip file created on Windows.
# It will append the dynamic (*.dylib) and static (*.a) libraries compiled on macOS
# into the zip file.

if [[ $# -eq 0 ]] ; then
    echo 'USAGE: ./update_packaged_plugin.sh <GAMEKIT_CPP_PATH> <ZIP_FILE>'
    echo 'Example: ./update_packaged_plugin.sh ~/workspace/AwsGameTechGdkCppSdk ~/temp/AwsGameKit.zip'
    exit 1
fi

currdir=$PWD
pushd $1

echo 'Building AWS GameKit for macOS (Debug)...'
rm -rf build_mac; mkdir build_mac; cd build_mac
../scripts/Mac/regenerate_projects_mac.sh ~/development/aws-sdk-cpp/AWSSDK_mac ~/development/boost_1_76_0 ~/development/yaml-cpp ~/development/googletest .. Debug
xcodebuild -parallelizeTargets -configuration Debug -target ALL_BUILD
xcodebuild -parallelizeTargets -configuration Debug -target install

echo 'Copying macOS Dynamic Libraries (Debug)...'
../scripts/Mac/refreshplugin_mac.sh $currdir Debug
cd ..

echo -e '\n----\n'
echo 'Building AWS GameKit for macOS (Release)...'
rm -rf build_mac; mkdir build_mac; cd build_mac
../scripts/Mac/regenerate_projects_mac.sh ~/development/aws-sdk-cpp/AWSSDK_mac ~/development/boost_1_76_0 ~/development/yaml-cpp ~/development/googletest .. Release
xcodebuild -parallelizeTargets -configuration Release -target ALL_BUILD
xcodebuild -parallelizeTargets -configuration Release -target install

echo 'Copying macOS Dynamic Libraries (Release)...'
../scripts/Mac/refreshplugin_mac.sh $currdir Release
cd ..

echo -e '\n----\n'
echo 'Building AWS GameKit for iOS (Debug)...'
rm -rf build_ios; mkdir build_ios; cd build_ios
../scripts/IOS/regenerate_projects_ios.sh ~/development/aws-sdk-cpp/AWSSDK_ios ~/development/boost_1_76_0 ~/development/yaml-cpp ~/development/Build-OpenSSL-cURL/curl ~/development/Build-OpenSSL-cURL/openssl/IOS ~/development/Build-OpenSSL-cURL/nghttp2/iOS/arm64 .. Debug
xcodebuild -parallelizeTargets -configuration Debug -target ALL_BUILD

echo 'Copying iOS Static Libraries (Debug)...'
../scripts/IOS/refreshplugin_ios.sh ~/development/aws-sdk-cpp/AWSSDK_ios ~/development/boost ~/development/yaml-cpp ~/development/Build-OpenSSL-cURL/curl ~/development/Build-OpenSSL-cURL/openssl ~/development/Build-OpenSSL-cURL/nghttp2 $currdir Debug
cd ..

echo -e '\n----\n'
echo 'Building AWS GameKit for iOS (Release)...'
rm -rf build_ios; mkdir build_ios; cd build_ios
../scripts/IOS/regenerate_projects_ios.sh ~/development/aws-sdk-cpp/AWSSDK_ios ~/development/boost_1_76_0 ~/development/yaml-cpp ~/development/Build-OpenSSL-cURL/curl ~/development/Build-OpenSSL-cURL/openssl/IOS ~/development/Build-OpenSSL-cURL/nghttp2/iOS/arm64 .. Release
xcodebuild -parallelizeTargets -configuration Release -target ALL_BUILD

echo 'Copying iOS Static Libraries (Release)...'
../scripts/IOS/refreshplugin_ios.sh ~/development/aws-sdk-cpp/AWSSDK_ios ~/development/boost ~/development/yaml-cpp ~/development/Build-OpenSSL-cURL/curl ~/development/Build-OpenSSL-cURL/openssl ~/development/Build-OpenSSL-cURL/nghttp2 $currdir Release
cd ..

echo -e '\n----\n'
echo 'Copying headers...'
./scripts/Mac/copyheaders.sh $currdir

echo -e '\n----\n'
echo 'Done building and copying binaries'

popd

echo -e '\n----\n'
echo 'Building plugin...'

"/Users/Shared/Epic Games/UE_4.27/Engine/Build/BatchFiles/RunUAT.sh" BuildPlugin -Plugin="$currdir/AwsGameKit/AwsGameKit.uplugin" -Package=$currdir/plugin_package/AwsGameKit -Rocket

pushd $currdir/plugin_package

echo -e '\n----\n'
echo 'Copying headers to final package...'
cp -p -v -R $currdir/AwsGameKit/Libraries/include/ ./AwsGameKit/Libraries/include/
zip -x \*.DS_Store -r -u $2 AwsGameKit/Libraries/include

echo -e '\n----\n'
echo "Adding Plugin Binaries to $2"
zip -x \*.DS_Store -r -u $2 AwsGameKit/Binaries/Mac

echo -e '\n----\n'
echo "Adding Plugin Intermediate directory to $2"
zip -x \*.DS_Store -r -u $2 AwsGameKit/Intermediate

echo -e '\n----\n'
echo "Adding macOS libraries to $2"
zip -x \*.DS_Store -r -u $2 AwsGameKit/Libraries/Mac

echo -e '\n----\n'
echo "Adding iOS libraries to $2"
zip -x \*.DS_Store -r -u $2 AwsGameKit/Libraries/IOS

popd

echo -e '\n----\n'
echo -e 'Done.\n'
