// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System;

namespace Microsoft.AspNetCore.Server.IIS.Core
{
    internal class IISNativeApplication
    {
        private readonly IntPtr _nativeApplication;

        public IISNativeApplication(IntPtr nativeApplication)
        {
            _nativeApplication = nativeApplication;
        }

        public void StopIncomingRequests()
        {
            NativeMethods.HttpStopIncomingRequests(_nativeApplication);
        }

        public void StopCallsIntoManaged()
        {
            NativeMethods.HttpStopCallsIntoManaged(_nativeApplication);
        }

        public void RegisterCallbacks(
            NativeMethods.PFN_REQUEST_HANDLER requestHandler,
            NativeMethods.PFN_SHUTDOWN_HANDLER shutdownHandler,
            NativeMethods.PFN_ASYNC_COMPLETION onAsyncCompletion,
            IntPtr requestContext,
            IntPtr shutdownContext)
        {
            NativeMethods.HttpRegisterCallbacks(_nativeApplication, requestHandler, shutdownHandler, onAsyncCompletion, requestContext, shutdownContext);
        }

        public void Dispose()
        {
            GC.SuppressFinalize(this);
        }

        ~IISNativeApplication()
        {
            // If this finalize is invoked, try our best to block all calls into managed.
            StopCallsIntoManaged();
        }
    }
}
