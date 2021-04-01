// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

/** @file
 * @brief Helpers for invoking function calls to/from the low level GameKit C APIs.
 */

#pragma once

/**
 * @brief A pointer to an instance of a class that can receive a callback.
 *
 * @details The callback method signature is specified by each API which uses a DISPATCH_RECEIVER_HANDLE.
 *
 * For example: AwsGameKitCoreWrapper::GameKitSettingsGetGameName() uses a callback signature of CharPtrCallback,
 * whereas AwsGameKitCoreWrapper::GameKitSettingsGetCustomEnvironments() uses a callback signature of KeyValueCharPtrCallbackDispatcher.
 */
typedef void* DISPATCH_RECEIVER_HANDLE;

/**
 * @brief A static dispatcher function pointer that receives a character array.
 *
 * @param dispatchReceiver A pointer to an instance of a class where the results will be dispatched to.
 * This instance must have a method signature of void ReceiveResult(const char* charPtr);
 * @param charPtr The character array pointer that the callback function receives.
*/
typedef void(*CharPtrCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* charPtr);

/**
 * @brief A static dispatcher function pointer that receives key/value pairs.
 *
 * @param dispatchReceiver A pointer to an instance of a class where the results will be dispatched to.
 * This instance must have have a method signature of void ReceiveResult(const char* charKey, const char* charValue);
 * @param charKey The key that the callback function receives.
 * @param charValue The value that the callback function receives.
*/
typedef void(*KeyValueCharPtrCallbackDispatcher)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* charKey, const char* charValue);


template <typename Functor, Functor> struct FunctorDispatcher;
template <typename Functor, typename RetType, typename ... Args, RetType(Functor::* CbFunc)(Args...)>
/**
 * @brief Templated struct that dispatches a call to a Functor object.
 *
 * @details Used to wrap member functions as function pointers so that they can be used as callbacks for GameKit low level APIs.
 * Example: if a low level GameKit Api has the form of
 *      void GameKitLowLevelSomeFunction(GAMEKIT_HANDLE handle, DISPATCH_RECEIVER_HANDLE receiver, CallbackFunc callback)
 * and CallbackFunc is
 *      typedef void(*CallbackFunc)(DISPATCH_RECEIVER_HANDLE receiver, Arg1 arg, Arg2 arg)
 *
 * Then declare a member function in a class of the form
 *      void Class::MemberFunction(Arg1 arg, Arg2 arg)
 *
 * Define a FunctorDispatcher typedef that wraps the member function
 *      typedef FunctorDispatcher<void(Class::*)(Arg1, Arg2), &Class::MemberFunction> MemberFunctionDispatcher;
 *
 * Then the MemberFunctionDispatcher can be used when calling the low level API:
 *      GameKitLowLevelSomeFunction(handle, this, &MemberFunctionDispatcher::Dispatch);
 */
struct FunctorDispatcher<RetType(Functor::*)(Args...), CbFunc>
{
    static RetType Dispatch(void* obj, Args... args)
    {
        Functor* instance = static_cast<Functor*>(obj);
        return (instance->*CbFunc)(std::forward<Args>(args)...);
    }

    static RetType Dispatch(void* obj, Args&&... args)
    {
        Functor* instance = static_cast<Functor*>(obj);
        return (instance->*CbFunc)(std::forward<Args>(args)...);
    }
};

template <typename Lambda, typename RetType, typename ... Args>
/**
 * @brief Templated struct that dispatches a call to a Lambda function.
 *
 * @details Used to wrap Lambdas as function pointers so that they can be used as callbacks for GameKit low level APIs.
 * Example: if a low level GameKit Api has the form of
 *      void GameKitLowLevelSomeFunction(GAMEKIT_HANDLE handle, DISPATCH_RECEIVER_HANDLE receiver, CallbackFunc callback)
 * and CallbackFunc is
 *      typedef void(*CallbackFunc)(DISPATCH_RECEIVER_HANDLE receiver, Arg1 arg, Arg2 arg)
 *
 * Then declare a member lambda of the form
 *      auto lambdaFunction = [](Arg1 arg1, Arg2 arg2) -> void { ... };
 *
 * Define a LambdaDispatcher typedef that wraps the lambda function
 *      typedef LambdaDispatcher<decltype(lambdaFunction), void, Arg1, Arg2> LambdaFunctionDispatcher;
 *
 * Then the LambdaFunctionDispatcher can be used when calling the low level API:
 *      GameKitLowLevelSomeFunction(handle, (void*)lambdaFunction, &LambdaFunctionDispatcher::Dispatch);
 */
struct LambdaDispatcher
{
    static RetType Dispatch(void* func, Args... args)
    {
        return (*static_cast<Lambda*>(func)) (std::forward<Args>(args)...);
    }

    static RetType Dispatch(void* func, Args&&... args)
    {
        return (*static_cast<Lambda*>(func)) (std::forward<Args>(args)...);
    }
};
