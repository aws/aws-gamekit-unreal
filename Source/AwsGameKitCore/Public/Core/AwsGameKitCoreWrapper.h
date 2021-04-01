// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitDispatcher.h"
#include "AwsGameKitLibraryUtils.h"
#include "AwsGameKitLibraryWrapper.h"
#include "AwsGameKitMarshalling.h"
#include "Logging.h"

// Standard Library
#include <string>

// Unreal
#include "Containers/UnrealString.h"

/**
 * Handle to an instance of GameKitAccount created inside the GameKit Identity DLL.
 *
 * An instance handle must be passed to all GameKit APIs which operate on a GameKitAccount class instance.
 *
 * Instance handles are stored as a void* (instead of a class type) because the GameKit DLLs expose a C-level interface (not a C++ interface).
 */
typedef void* GAMEKIT_ACCOUNT_INSTANCE_HANDLE;

/**
 * Handle to an instance of GameKitResources created inside the GameKit Identity DLL.
 *
 * An instance handle must be passed to all GameKit APIs which operate on a GameKitResources class instance.
 *
 * Instance handles are stored as a void* (instead of a class type) because the GameKit DLLs expose a C-level interface (not a C++ interface).
 */
typedef void* GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE;

/**
 * Handle to an instance of GameKitSettings created inside the GameKit Identity DLL.
 *
 * An instance handle must be passed to all GameKit APIs which operate on a GameKitSettings class instance.
 *
 * Instance handles are stored as a void* (instead of a class type) because the GameKit DLLs expose a C-level interface (not a C++ interface).
 */
typedef void* GAMEKIT_SETTINGS_INSTANCE_HANDLE;

// Logging callback
typedef void(*FuncLogCallback)(unsigned int level, const char* message, int size);

// Describe resources callback
typedef void(*FuncResourceInfoCallback)(const char* logicalResourceId, const char* resourceType, const char* resourceStatus);

/**
 * This class exposes the GameKit Core APIs and loads the underlying DLL into memory.
 *
 * This is a barebones wrapper over the DLL's C-level interface. It uses C data types instead of Unreal data types (ex: char* instead of FString).
 *
 * This class exposes three underlying GameKit classes: GameKitAccount, GameKitResources, and GameKitSettings:
 *
 * == GameKitAccount ==
 * Offers plugin-level, AWS account-level, and cross-feature methods.
 *
 * The feature-related methods operate on ALL features at the same time.
 * To work on a single feature, use GameKitFeatureResources instead.
 *
 * For example, GameKitAccount can:
 * - Get path to the plugin's installation directory.
 * - Get path to the base resources and instance resources.
 * - Create instance resource files.
 * - Create the bootstrap S3 bucket.
 * - Deploy the main stack and all feature stacks.
 * - Deploy the shared API Gateway stage.
 * - Save secrets to AWS Secrets Manager.
 *
 * == GameKitResources ==
 * Offers methods for working on the AWS resources of a single GameKit feature (ex: "achievements").
 *
 * Each instance of this class operates on one feature, which is specified in it's constructor.
 *
 * For example, it can deploy/delete the feature's CloudFormation stack and create it's instance template files for deployment.
 *
 * == GameKitSettings ==
 * Offers read/write access to the "saveInfo.yml" GameKit Settings file.
 *
 * The settings file contains information such as:
 * - Game name
 * - Custom deployment environments (ex: "Gamma")
 * - List of activated/deactivated features
 * - Feature-specific variables (ex: "isFacebookLoginEnabled" for Identity)
 *
 * The file is stored at "GAMEKIT_ROOT/shortGameName/saveInfo.yml".
 *
 * The file is read/written through usage of the plugin UI. For example, when a feature is activated,
 * when feature variables are filled in, or when a custom deployment environment is added or removed.
 *
 * The file is loaded during the constructor, and can be reloaded by calling Reload().
 * Call SaveSettings() to write the settings to back to disk after making modifications with any "Set", "Add/Delete", "Activate/Deactivate" methods.
 */

class AWSGAMEKITCORE_API AwsGameKitCoreWrapper : public AwsGameKitLibraryWrapper
{
private:
    /**
    * GameKit static function pointer handles
    */

    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetAwsAccountId, (void* caller, CharPtrCallback resultCallback, const char* accessKey, const char* secretKey, FuncLogCallback logCb));

    /**
    * GameKit Account function pointer handles
    */

    DEFINE_FUNC_HANDLE(void*, GameKitAccountInstanceCreate, (AccountInfo accountInfo, AccountCredentials credentials, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void*, GameKitAccountInstanceCreateWithRootPaths, (AccountInfo accountInfo, AccountCredentials credentials, const char* rootPath, const char* pluginRootPath, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void, GameKitAccountInstanceRelease, (void* accountInstance));;
    DEFINE_FUNC_HANDLE(const char*, GameKitAccountGetRootPath, (void* accountInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitAccountGetPluginRootPath, (void* accountInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitAccountGetBaseCloudFormationPath, (void* accountInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitAccountGetBaseFunctionsPath, (void* accountInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitAccountGetInstanceCloudFormationPath, (void* accountInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitAccountGetInstanceFunctionsPath, (void* accountInstance));
    DEFINE_FUNC_HANDLE(void, GameKitAccountSetRootPath, (void* accountInstance, const char* rootPath));
    DEFINE_FUNC_HANDLE(void, GameKitAccountSetPluginRootPath, (void* accountInstance, const char* pluginRootPath));
    DEFINE_FUNC_HANDLE(bool, GameKitAccountHasValidCredentials, (void* accountInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountInstanceBootstrap, (void* accountInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountSaveSecret, (void* accountInstance, const char* secretName, const char* secretValue));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountCheckSecretExists, (void* accountInstance, const char* secretName));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountSaveFeatureInstanceTemplates, (void* accountInstance, const char* const* varKeys, const char* const* varValues, int numKeys));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountUploadAllDashboards, (void* accountInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountUploadLayers, (void* accountInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountUploadFunctions, (void* accountInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountCreateOrUpdateMainStack, (void* accountInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountCreateOrUpdateStacks, (void* accountInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAccountDeployApiGatewayStage,(void* accountInstance));

    /**
    * GameKit Resources function pointer handles
    */

    DEFINE_FUNC_HANDLE(void*, GameKitResourcesInstanceCreate, (AccountInfo accountInfo, AccountCredentials credentials, FeatureType featureType, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void*, GameKitResourcesInstanceCreateWithRootPaths, (AccountInfo accountInfo, AccountCredentials credentials, FeatureType featureType, const char* rootPath, const char* pluginRootPath, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void, GameKitResourcesInstanceRelease, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitResourcesGetRootPath, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitResourcesGetPluginRootPath, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitResourcesGetBaseCloudFormationPath, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitResourcesGetBaseFunctionsPath, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitResourcesGetInstanceCloudFormationPath, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitResourcesGetInstanceFunctionsPath, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(void, GameKitResourcesSetRootPath, (void* resourceInstance, const char* rootPath));
    DEFINE_FUNC_HANDLE(void, GameKitResourcesSetPluginRootPath, (void* resourceInstance, const char* pluginRootPath));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesInstanceCreateOrUpdateStack, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesInstanceDeleteStack, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(const char*, GameKitResourcesGetCurrentStackStatus, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(bool, GameKitResourcesIsCloudFormationInstanceTemplatePresent, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesSaveDeployedCloudFormationTemplate, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesSaveCloudFormationInstance, (void* resourceInstance, const char* const* varKeys, const char* const* varValues, int numKeys));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesUpdateCloudFormationParameters, (void* resourceInstance, const char* const* varKeys, const char* const* varValues, int numKeys));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesSaveLayerInstances, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesSaveFunctionInstances, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesUploadFeatureLayers, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesUploadFeatureFunctions, (void* resourceInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitResourcesDescribeStackResources, (void* resourceInstance, FuncResourceInfoCallback resourceInfoCb));

    /**
    * GameKit Settings function pointer handles
    */

    DEFINE_FUNC_HANDLE(void*, GameKitSettingsInstanceCreate, (const char* rootPath, const char* pluginVersion, const char* shortGameName, const char* currentEnvironment, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsInstanceRelease, (void* settingsInstance));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsSetGameName, (void* settingsInstance, const char* gameName));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsSetLastUsedRegion, (void* settingsInstance, const char* region));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsSetLastUsedEnvironment, (void* settingsInstance, const char* envCode));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsAddCustomEnvironment, (void* settingsInstance, const char* envCode, const char* envDescription));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsDeleteCustomEnvironment, (void* settingsInstance, const char* envCode));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsActivateFeature, (void* settingsInstance, FeatureType featureType));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsDeactivateFeature, (void* settingsInstance, FeatureType featureType));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsSetFeatureVariables, (void* settingsInstance, FeatureType featureType, const char* const* varKeys, const char* const* varValues, size_t numKeys));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsDeleteFeatureVariable, (void* settingsInstance, FeatureType featureType, const char* varName));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitSettingsSave, (void* settingsInstance));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsGetGameName, (void* settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsGetLastUsedRegion, (void* settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsGetLastUsedEnvironment, (void* settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsGetCustomEnvironments, (void* settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, KeyValueCharPtrCallbackDispatcher resultsCb));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsGetCustomEnvironmentDescription, (void* settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, const char* envCode, CharPtrCallback resultsCb));
    DEFINE_FUNC_HANDLE(bool, GameKitSettingsIsFeatureActive, (void* settingsInstance, FeatureType featureType));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsGetFeatureVariables, (void* settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, FeatureType featureType, KeyValueCharPtrCallbackDispatcher resultsCb));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsGetFeatureVariable, (void* settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, FeatureType featureType, const char* varName, CharPtrCallback resultsCb));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsGetSettingsFilePath, (void* settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb));
    DEFINE_FUNC_HANDLE(void, GameKitSettingsReload, (void* settingsInstance));

protected:
    virtual std::string getLibraryFilename() override
    {
        return "aws-gamekit-core";
    }

    virtual void importFunctions(void* loadedDllHandle) override;

public:
#pragma region GameKitAccount
    // -------- Static functions, these don't require a GameKitAccount instance handle
    /**
     * @brief Get the AWS Account ID which corresponds to the provided Access Key and Secret Key.
     *
     * @details For more information about AWS access keys and secret keys, see: https://docs.aws.amazon.com/general/latest/gr/aws-sec-cred-types.html#access-keys-and-secret-access-keys
     *
     * @param caller Pointer to the caller object (object that will handle the callback function).
     * @param resultCb Pointer to the callback function to invoke on completion.
     * @param accessKey The AWS Access Key.
     * @param secretKey The AWS Secret Key.
     * @param logCb Callback function for logging information and errors.
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitGetAwsAccountId(DISPATCH_RECEIVER_HANDLE caller, CharPtrCallback resultCb, const char* accessKey, const char* secretKey, FuncLogCallback logCb);

    // -------- Instance functions, these require a GameKitAccount instance handle
    /**
     * @brief Create a GameKitAccount instance, which can be used to access the GameKitAccount API.
     *
     * @details Make sure to call GameKitAccountInstanceRelease() to destroy the returned object when finished with it.
     *
     * @param accountInfo Struct holding account id, game name, and deployment environment.
     * @param credentials Struct holding account id, region, access key, and secret key.
     * @param logCb Callback function for logging information and errors.
     * @return Pointer to the new GameKitAccount instance.
     */
    virtual GAMEKIT_ACCOUNT_INSTANCE_HANDLE GameKitAccountInstanceCreate(AccountInfo accountInfo, AccountCredentials credentials, FuncLogCallback logCb);

    /**
      * @brief Create a GameKitAccount instance, which can be used to access the GameKitAccount API. Also sets the plugin and game root paths.
      *
      * @details Make sure to call GameKitAccountInstanceRelease() to destroy the returned object when finished with it.
      *
      * @param accountInfo Struct holding account id, game name, and deployment environment.
      * @param credentials Struct holding account id, region, access key, and secret key.
      * @param rootPath New path for GAMEKIT_ROOT
      * @param pluginRootPath New path for the plugin root directory.
      * @param logCb Callback function for logging information and errors.
      * @return Pointer to the new GameKitAccount instance.
      */
    virtual GAMEKIT_ACCOUNT_INSTANCE_HANDLE GameKitAccountInstanceCreateWithRootPaths(AccountInfo accountInfo, AccountCredentials credentials, const char* rootPath, const char* pluginRootPath, FuncLogCallback logCb);

    /**
     * @brief Destroy the provided GameKitAccount instance.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     */
    virtual void GameKitAccountInstanceRelease(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Get the GAMEKIT_ROOT path where the "instance" templates and settings are going to be stored.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return GAMEKIT_ROOT path, or an empty string if GameKitAccountSetRootPath() hasn't been called yet.
     */
    virtual const char* GameKitAccountGetRootPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Set the GAMEKIT_ROOT path where the "instance" templates and settings are going to be stored.
     *
     * @details This value can be fetched by GameKitAccountGetRootPath() and defaults to an empty string until this method is called.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @param rootPath New path for GAMEKIT_ROOT.
     */
    virtual void GameKitAccountSetRootPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* rootPath);

    /**
     * @brief Get the root directory of the GameKit plugin's installation.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return Plugin root path, or an empty string if GameKitAccountSetPluginRootPath() hasn't been called yet.
     */
    virtual const char* GameKitAccountGetPluginRootPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Set the root directory where the GameKit plugin was installed.
     *
     * @details This value can be fetched by GameKitAccountGetPluginRootPath() and defaults to an empty string until this method is called.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @param pluginRootPath New path for the plugin root directory.
     */
    virtual void GameKitAccountSetPluginRootPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* pluginRootPath);

    /**
     * @brief Get the path where the "base" CloudFormation templates are stored.
     *
     * @details This path is set by calling GameKitAccountSetPluginRootPath(), and is equal to GameKitAccountGetPluginRootPath()+ResourceDirectories::CLOUDFORMATION_DIRECTORY.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return Path to the "base" CloudFormation templates, or an empty string if GameKitAccountSetPluginRootPath() hasn't been called yet.
     */
    virtual const char* GameKitAccountGetBaseCloudFormationPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Get the path where the "base" Lambda functions are stored.
     *
     * @details This path is set by calling GameKitAccountSetPluginRootPath(), and is equal to GameKitAccountGetPluginRootPath()+ResourceDirectories::FUNCTIONS_DIRECTORY.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return Path to the "base" Lambda functions, or an empty string if GameKitAccountSetPluginRootPath() hasn't been called yet.
     */
    virtual const char* GameKitAccountGetBaseFunctionsPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Get the path where the "instance" CloudFormation templates are stored.
     *
     * @details This path is set by calling GameKitAccountSetRootPath(), and is equal to GameKitAccountGetRootPath()+<gameName>+ResourceDirectories::CLOUDFORMATION_DIRECTORY;.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return Path to the "instance" CloudFormation templates, or an empty string if GameKitAccountSetRootPath() hasn't been called yet.
     */
    virtual const char* GameKitAccountGetInstanceCloudFormationPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Get the path where the "instance" Lambda functions are stored.
     *
     * @details This path is set by calling GameKitAccountSetRootPath(), and is equal to GameKitAccountGetRootPath()+<gameName>+ResourceDirectories::FUNCTIONS_DIRECTORY.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return Path to the "instance" Lambda functions, or an empty string if GameKitAccountSetPluginRootPath() hasn't been called yet.
     */
    virtual const char* GameKitAccountGetInstanceFunctionsPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Return True if the provided GameKitAccount instance has valid AWS credentials (access key, secret key, and AWS region), return False otherwise.
     *
     * @details In this case, "valid" means the credentials are allowed to list S3 buckets (i.e. the IAM Role has "s3:ListAllMyBuckets" permission).
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return True if the credentials are valid, false otherwise.
     */
    virtual bool GameKitAccountHasValidCredentials(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Create a bootstrap bucket in the AWS account if it doesn't already exist.
     *
     * @details The bootstrap bucket must be created before deploying any stacks or Lambda functions.
     * There needs to be a unique bootstrap bucket for each combination of Environment, Account ID, and GameName.
     *
     * @details The bucket name will be "do-not-delete-gamekit-<environment>-<accountId>-<gameName>"
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitAccountInstanceBootstrap(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Create or update a secret in AWS SecretsManager (https://aws.amazon.com/secrets-manager/).
     *
     * @details The secret name will be "gamekit_<environment>_<gameName>_<secretName>", for example: "gamekit_dev_mygame_amazon_client_secret".
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @param secretName Name of the secret. Will be prefixed as described in the details.
     * @param secretValue Value of the secret.
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitAccountSaveSecret(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* secretName, const char* secretValue);

    /**
     * @brief Checks if a secret exists in AWS SecretsManager (https://aws.amazon.com/secrets-manager/).
     *
     * @details The secret name will be "gamekit_<environment>_<gameName>_<secretName>", for example: "gamekit_dev_mygame_amazon_client_secret".
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @param secretName Name of the secret. Will be prefixed as described in the details.
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult errors.h file for details.
     */
    virtual unsigned int GameKitAccountCheckSecretExists(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* secretName);

    /**
     * @brief Create or overwrite the instance template files for every feature.
     *
     * @details The parameters "varKeys", "varValues", and "numKeys" represent a map<string, string>, where varKeys[N] maps to varValues[N], and numKeys is the total number of key-value pairs.
     * The key-value pairs are used to replace placeholder variables prefixed with "AWSGAMEKIT::VARS::" in the CloudFormation templates and CloudFormation parameters files.
     *
     * @details Creates the following instance files for each feature: CloudFormation template, CloudFormation parameters, and Lambda functions.
     * Instance files are written to the "GAMEKIT_ROOT" path, and are created as copies of the "Base" path files with the placeholder variables filled in.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @param varKeys Keys of the placeholders to replace. For example, "my_key" in the following placeholder: "AWSGAMEKIT::VARS::my_key".
     * @param varValues Values to replace the placeholders with.
     * @param numKeys The number of key-value pairs. The length of varKeys, varValues, and numKeys should be equal.
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitAccountSaveFeatureInstanceTemplates(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* const* varKeys, const char* const* varValues, int numKeys);

    /**
     * @brief Upload the dashboard configuration file for every feature to the bootstrap bucket.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitAccountUploadAllDashboards(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Upload the Lambda instance layers for every feature to the bootstrap bucket.
     *
     * @details GameKitAccountSaveFeatureInstanceTemplates() should be called first to create the Lambda instance templates.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitAccountUploadLayers(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Upload the Lambda instance functions for every feature to the bootstrap bucket.
     *
     * @details GameKitAccountSaveFeatureInstanceTemplates() should be called first to create the Lambda instance templates.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitAccountUploadFunctions(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @deprecated
     *
     * @brief Deploy the "main" CloudFormation stack to AWS.
     *
     * @details Should call GameKitAccountSaveFeatureInstanceTemplates() first to create the instance templates.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitAccountCreateOrUpdateMainStack(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Deploy all CloudFormation stacks to AWS (i.e. the "main" stack and all feature stacks).
     *
     * @details Should call GameKitAccountSaveFeatureInstanceTemplates() first to create the instance templates.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitAccountCreateOrUpdateStacks(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);

    /**
     * @brief Deploy the API Gateway stage of the "main" CloudFormation stack.
     *
     * @param accountInstance Pointer to a GameKitAccount instance created with GameKitAccountInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitAccountDeployApiGatewayStage(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance);
#pragma endregion

#pragma region GameKitFeatureResources
    /**
     * @brief Create a GameKitFeatureResources instance, which can be used to access the GameKitFeatureResources API.
     *
     * @details Make sure to call GameKitResourcesInstanceRelease() to destroy the returned object when finished with it.
     *
     * @param accountInfo Struct holding account id, game name, and deployment environment.
     * @param credentials Struct holding account id, region, access key, and secret key.
     * @param featureType The GameKit feature to work with.
     * @param logCb Callback function for logging information and errors.
     * @return Pointer to the new GameKitFeatureResources instance.
     */
    virtual GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE GameKitResourcesInstanceCreate(AccountInfo accountInfo, AccountCredentials credentials, FeatureType featureType, FuncLogCallback logCb);

    /**
      * @brief Create a GameKitFeatureResources instance, which can be used to access the GameKitFeatureResources API. Also sets the root and pluginRoot paths.
      *
      * @details Make sure to call GameKitResourcesInstanceRelease() to destroy the returned object when finished with it.
      *
      * @param accountInfo Struct holding account id, game name, and deployment environment.
      * @param credentials Struct holding account id, region, access key, and secret key.
      * @param featureType The GameKit feature to work with.
      * @param rootPath New path for GAMEKIT_ROOT.
      * @param pluginRootPath New path for the plugin root directory.
      * @param logCb Callback function for logging information and errors.
      * @return Pointer to the new GameKitFeatureResources instance.
      */
    virtual GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE GameKitResourcesInstanceCreateWithRootPaths(AccountInfo accountInfo, AccountCredentials credentials, FeatureType featureType, const char* rootPath, const char* pluginRootPath, FuncLogCallback logCb);

    /**
     * @brief Destroy the provided GameKitFeatureResources instance.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     */
    virtual void GameKitResourcesInstanceRelease(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Get the GAMEKIT_ROOT path where the "instance" templates and settings are going to be stored.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return GAMEKIT_ROOT path, or an empty string if GameKitResourcesSetRootPath() hasn't been called yet.
     */
    virtual const char* GameKitResourcesGetRootPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Set the GAMEKIT_ROOT path where the "instance" templates and settings are going to be stored.
     *
     * @details This value can be fetched by GameKitResourcesGetRootPath() and defaults to an empty string until this method is called.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @param rootPath New path for GAMEKIT_ROOT.
     */
    virtual void GameKitResourcesSetRootPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char* rootPath);

    /**
     * @brief Get the root directory of the GameKit plugin's installation.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return Plugin root path, or an empty string if GameKitResourcesSetPluginRootPath() hasn't been called yet.
     */
    virtual const char* GameKitResourcesGetPluginRootPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Set the root directory where the GameKit plugin was installed.
     *
     * @details This value can be fetched by GameKitResourcesGetPluginRootPath() and defaults to an empty string until this method is called.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @param pluginRootPath New path for the plugin root directory.
     */
    virtual void GameKitResourcesSetPluginRootPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char* pluginRootPath);

    /**
     * @brief Get the path where this feature's "base" CloudFormation template is stored.
     *
     * @details This path is set by calling GameKitResourcesSetPluginRootPath(), and is equal to GameKitResourcesGetPluginRootPath()+ResourceDirectories::CLOUDFORMATION_DIRECTORY+<feature_name>.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return Path to this feature's "base" CloudFormation template, or an empty string if GameKitResourcesSetPluginRootPath() hasn't been called yet.
     */
    virtual const char* GameKitResourcesGetBaseCloudFormationPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Get the path where this feature's "base" Lambda functions are stored.
     *
     * @details This path is set by calling GameKitResourcesSetPluginRootPath(), and is equal to GameKitResourcesGetPluginRootPath()+ResourceDirectories::FUNCTIONS_DIRECTORY+<feature_name>.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return Path to this feature's "base" Lambda functions, or an empty string if GameKitResourcesGetPluginRootPath() hasn't been called yet.
     */
    virtual const char* GameKitResourcesGetBaseFunctionsPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Get the path where this feature's "instance" CloudFormation template is stored.
     *
     * @details This path is set by calling GameKitResourcesSetRootPath(), and is equal to GameKitResourcesGetRootPath()+<gameName>+ResourceDirectories::CLOUDFORMATION_DIRECTORY+<feature_name>;.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return Path to this feature's "instance" CloudFormation template, or an empty string if GameKitResourcesGetRootPath() hasn't been called yet.
     */
    virtual const char* GameKitResourcesGetInstanceCloudFormationPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Get the path where this feature's "instance" Lambda functions are stored.
     *
     * @details This path is set by calling GameKitResourcesSetRootPath(), and is equal to GameKitResourcesGetRootPath()+<gameName>+ResourceDirectories::FUNCTIONS_DIRECTORY+<feature_name>.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return Path to this feature's "instance" Lambda functions, or an empty string if GameKitResourcesGetRootPath() hasn't been called yet.
     */
    virtual const char* GameKitResourcesGetInstanceFunctionsPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Deploy this feature's CloudFormation stack to AWS.
     *
     * @details Deploys the instance CloudFormation template. Creates a new stack if no stack exists, or updates an existing stack.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitResourcesInstanceCreateOrUpdateStack(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Delete this feature's CloudFormation stack from AWS.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitResourcesInstanceDeleteStack(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Get the status of this feature's deployed CloudFormation stack, such as "CREATE_COMPLETE" or "UPDATE_IN_PROGRESS".
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return The stack status string, or "UNDEPLOYED" if the stack is not deployed.
     */
    virtual const char* GameKitResourcesGetCurrentStackStatus(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Checks if feature's CloudFormation template is present in the feature's instance directory.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return true if the feature's CloudFormation template is present in the feature's instance directory.
     */
    virtual bool GameKitResourcesIsCloudFormationInstanceTemplatePresent(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Retrieves the feature's deployed CloudFormation template and saves it to the feature's instance directory.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult errors.h file for details.
     */
    virtual unsigned int GameKitResourcesSaveDeployedCloudFormationTemplate(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Create or overwrite this feature's instance CloudFormation template file and CloudFormation parameters file.
     *
     * @details The parameters "varKeys", "varValues", and "numKeys" represent a map<string, string>, where varKeys[N] maps to varValues[N], and numKeys is the total number of key-value pairs.
     * The key-value pairs are used to replace placeholder variables prefixed with "AWSGAMEKIT::VARS::" in the CloudFormation templates and CloudFormation parameters files.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @param varKeys Keys of the placeholders to replace. For example, "my_key" in the following placeholder: "AWSGAMEKIT::VARS::my_key".
     * @param varValues Values to replace the placeholders with.
     * @param numKeys The number of key-value pairs. The length of varKeys, varValues, and numKeys should be equal.
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitResourcesSaveCloudFormationInstance(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char* const* varKeys, const char* const* varValues, int numKeys);

    /**
     * @brief Update this feature's instance CloudFormation parameters file.
     *
     * @details The parameters "varKeys", "varValues", and "numKeys" represent a map<string, string>, where varKeys[N] maps to varValues[N], and numKeys is the total number of key-value pairs.
     * The key-value pairs are used to replace placeholder variables prefixed with "AWSGAMEKIT::VARS::" in the CloudFormation parameters files.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @param varKeys Keys of the placeholders to replace. For example, "my_key" in the following placeholder: "AWSGAMEKIT::VARS::my_key".
     * @param varValues Values to replace the placeholders with.
     * @param numKeys The number of key-value pairs. The length of varKeys, varValues, and numKeys should be equal.
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitResourcesUpdateCloudFormationParameters(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char* const* varKeys, const char* const* varValues, int numKeys);

    /**
     * @brief Create or overwrite this feature's instance Lambda layer files.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitResourcesSaveLayerInstances(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Create or overwrite this feature's instance Lambda function files.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitResourcesSaveFunctionInstances(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Upload this feature's instance Lambda layers to the S3 bootstrap bucket.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitResourcesUploadFeatureLayers(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Upload this feature's instance Lambda functions to the S3 bootstrap bucket.
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitResourcesUploadFeatureFunctions(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance);

    /**
     * @brief Log the current status of all the resources in this feature's CloudFormation stack.
     *
     * @details For example, a resource's status may look like: "CognitoLambdaRole, AWS::IAM::Role, CREATE_COMPLETE".
     *
     * @param resourceInstance Pointer to a GameKitFeatureResources instance created with GameKitResourcesInstanceCreate().
     * @param resourceInfoCb Callback function for logging the resources statuses.
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitResourcesDescribeStackResources(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, FuncResourceInfoCallback resourceInfoCb);
#pragma endregion

#pragma region GameKitSettings
    /**
     * @brief Create a GameKitSettings instance and load the settings from the GameKit Settings YAML file.
     *
     * @details Make sure to call GameKitSettingsInstanceRelease() to destroy the returned object when finished with it.
     *
     * @param rootPath The GAMEKIT_ROOT path where the "instance" templates and settings are stored.
     * @param pluginVersion The GameKit plugin version.
     * @param currentEnvironment The current active environment based on what was selected in ControlCenter eg "dev", "qa", custom
     * @param shortGameName A shortened version of the game name.
     * @param logCb Callback function for logging information and errors.
     * @return Pointer to the new GameKitSettings instance.
     */
    virtual GAMEKIT_SETTINGS_INSTANCE_HANDLE GameKitSettingsInstanceCreate(const char* rootPath, const char* pluginVersion, const char* shortGameName, const char* currentEnvironment, FuncLogCallback logCb);

    /**
     * @brief Destroy the provided GameKitSettings instance.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     */
    virtual void GameKitSettingsInstanceRelease(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance);

    /**
     * @brief Set the game's name.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param gameName The new name.
     */
    virtual void GameKitSettingsSetGameName(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* gameName);

    /**
     * @brief Set the last used region.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param region The region last used, example: "us-west-2".
     */
    virtual void GameKitSettingsSetLastUsedRegion(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* region);

    /**
     * @brief Set the last used environment.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param envCode The environment code.
     */
    virtual void GameKitSettingsSetLastUsedEnvironment(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* envCode);

    /**
     * @brief Add a custom deployment environment to the AWS Control Center menu.
     *
     * @details This custom environment will be available to select from the dropdown menu in the plugin's AWS Control Center,
     * alongside the default environments of "Development", "QA", "Staging", and "Production".
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param envCode Three letter code for the environment name. This code will be prefixed on all AWS resources that are
     * deployed to this environment. Ex: "gam" for "Gamma".
     * @param envDescription The environment name that will be displayed in the AWS Control Center. Ex: "Gamma".
     */
    virtual void GameKitSettingsAddCustomEnvironment(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* envCode, const char* envDescription);

    /**
     * @brief Remove a custom deployment environment from the AWS Control Center menu.
     *
     * @details Note: If you intend to delete the stacks deployed to this environment, you should delete them first
     * before deleting the custom environment. Otherwise you'll have to manually delete them from the AWS CloudFormation webpage.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param envCode Three letter code for the custom environment to delete. Ex: "gam" for "Gamma".
     */
    virtual void GameKitSettingsDeleteCustomEnvironment(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* envCode);

    /**
     * @brief Activate a GameKit feature (ex: "Achievements").
     *
     * @details After activating, the feature will be available to configure and deploy.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param featureType The feature to activate.
     */
    virtual void GameKitSettingsActivateFeature(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType);

    /**
     * @brief Deactivate a GameKit feature (ex: "Achievements").
     *
     * @details After deactivating, the feature will not be able to be configured or deployed.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param featureType The feature to deactivate.
     */
    virtual void GameKitSettingsDeactivateFeature(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType);

    /**
     * @brief Add key-value pairs to the feature's variables map, overwriting existing keys.
     *
     * @details The parameters "varKeys", "varValues", and "numKeys" represent a map<string, string>, where varKeys[N] maps to varValues[N], and numKeys is the total number of key-value pairs.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param featureType The feature to set the variables for.
     * @param varKeys The variable names.
     * @param varValues The variable values.
     * @param numKeys The number of key-value pairs. The length of varKeys, varValues, and numKeys should be equal.
     */
    virtual void GameKitSettingsSetFeatureVariables(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType, const char* const* varKeys, const char* const* varValues, size_t numKeys);

    /**
     * @brief Delete a key-value pair from the feature's variables map.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param featureType The feature to delete the variable from.
     * @param varName The variable name to delete.
     */
    virtual void GameKitSettingsDeleteFeatureVariable(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType, const char* varName);

    /**
     * @brief Write the GameKit Settings YAML file to disk.
     *
     * @details Call this to persist any changes made through the "Set", "Add/Delete", "Activate/Deactivate" methods.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @return The result code of the operation. GAMEKIT_SUCCESS if successful, else a non-zero value in case of error. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitSettingsSave(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance);

    /**
     * @brief Get the game's full name, example: "My Full Game Name".
     *
     * @details The game name is returned through the callback and receiver.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param receiver A pointer to an instance of a class where the results will be dispatched to.
     * This instance must have a method signature of void ReceiveResult(const char* charPtr);
     * @param resultsCb A static dispatcher function pointer that receives a character array.
     */
    virtual void GameKitSettingsGetGameName(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb);

    /**
     * @brief Get the developers last submitted region, example: "us-west-2".
     *
     * @details The region is returned through the callback and receiver.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param receiver A pointer to an instance of a class where the results will be dispatched to.
     * This instance must have a method signature of void ReceiveResult(const char* charPtr);
     * @param resultsCb A static dispatcher function pointer that receives a character array.
     */
    virtual void GameKitSettingsGetLastUsedRegion(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb);

    /**
     * @brief Get the developers last submitted environment code, example: "dev".
     *
     * @details The environment code is returned through the callback and receiver.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param receiver A pointer to an instance of a class where the results will be dispatched to.
     * This instance must have a method signature of void ReceiveResult(const char* charPtr);
     * @param resultsCb A static dispatcher function pointer that receives a character array.
     */
    virtual void GameKitSettingsGetLastUsedEnvironment(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb);

    /**
     * @brief Get all the custom environment key-value pairs (ex: "gam", "Gamma").
     *
     * @details The custom environments are returned through the callback and receiver.
     * The callback is invoked once for each custom environment.
     * The returned keys are 3-letter environment codes (ex: "gam"), and the values are corresponding environment descriptions (ex: "Gamma").
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param receiver A pointer to an instance of a class where the results will be dispatched to.
     * This instance must have have a method signature of void ReceiveResult(const char* charKey, const char* charValue);
     * @param resultsCb A static dispatcher function pointer that receives key/value pairs.
     */
    virtual void GameKitSettingsGetCustomEnvironments(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, KeyValueCharPtrCallbackDispatcher resultsCb);

    /**
     * @brief Get the custom environment description (ex: "Gamma") for the provided environment code (ex: "gam").
     *
     * @details The environment description is returned through the callback and receiver.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param receiver A pointer to an instance of a class where the results will be dispatched to.
     * This instance must have a method signature of void ReceiveResult(const char* charPtr);
     * @param envCode The 3-letter environment code to get the description for.
     * @param resultsCb A static dispatcher function pointer that receives a character array.
     */
    virtual void GameKitSettingsGetCustomEnvironmentDescription(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, const char* envCode, CharPtrCallback resultsCb);

    /**
     * @brief Return True if the feature is active, return false if not active.
     *
     * @details A feature can be activated/deactivated with the functions: GameKitSettingsActivateFeature() and GameKitSettingsDeactivateFeature()
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param featureType The feature to check.
     * @return True if the feature is active, false if not active.
     */
    virtual bool GameKitSettingsIsFeatureActive(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType);

    /**
     * @brief Get all of the feature's variables as key-value pairs.
     *
     * @details The variables are returned through the callback and receiver.
     * The callback is invoked once for each variable. The variables are returned as key-value pairs of (variableName, variableValue).
     * The callback will not be invoked if the feature is missing from the settings file.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param receiver A pointer to an instance of a class where the results will be dispatched to.
     * This instance must have have a method signature of void ReceiveResult(const char* charKey, const char* charValue);
     * @param featureType The feature to get the variables for.
     * @param resultsCb A static dispatcher function pointer that receives key/value pairs.
     */
    virtual void GameKitSettingsGetFeatureVariables(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, FeatureType featureType, KeyValueCharPtrCallbackDispatcher resultsCb);

    /**
     * @brief Get the value of the specified feature's variable.
     *
     * @details The value is returned through the callback and receiver.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param receiver A pointer to an instance of a class where the results will be dispatched to.
     * This instance must have a method signature of void ReceiveResult(const char* charPtr);
     * @param featureType The feature to get the variable for.
     * @param varName The name of the variable to get the value for.
     * @param resultsCb A static dispatcher function pointer that receives a character array.
     */
    virtual void GameKitSettingsGetFeatureVariable(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, FeatureType featureType, const char* varName, CharPtrCallback resultsCb);

    /**
     * @brief Get the path to the "saveInfo.yml" settings file.
     *
     * @details The path is equal to "GAMEKIT_ROOT/shortGameName/saveInfo.yml".
     * The path is returned through the callback and receiver.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     * @param receiver A pointer to an instance of a class where the results will be dispatched to.
     * This instance must have a method signature of void ReceiveResult(const char* charPtr);
     * @param resultsCb A static dispatcher function pointer that receives a character array.
     */
    virtual void GameKitSettingsGetSettingsFilePath(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb);

    /**
     * @brief Reload the GameKit Settings YAML file from disk.
     *
     * @details Overwrites any changes made through the "Set", "Add/Delete", "Activate/Deactivate" methods.
     *
     * @param settingsInstance Pointer to a GameKitSettings instance created with GameKitSettingsInstanceCreate().
     */
    virtual void GameKitSettingsReload(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance);
#pragma endregion
};
