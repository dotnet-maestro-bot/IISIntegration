// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System;
using System.Runtime.InteropServices;
using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.AspNetCore.Hosting.Server;
using Microsoft.AspNetCore.Server.IIS;
using Microsoft.AspNetCore.Server.IIS.Core;

namespace Microsoft.AspNetCore.Hosting
{
    public static class WebHostBuilderIISExtensions
    {
        /// <summary>
        /// Configures the port and base path the server should listen on when running behind AspNetCoreModule.
        /// The app will also be configured to capture startup errors.
        /// </summary>
        /// <param name="hostBuilder"></param>
        /// <returns></returns>
        public static IWebHostBuilder UseIIS(this IWebHostBuilder hostBuilder)
        {
            if (hostBuilder == null)
            {
                throw new ArgumentNullException(nameof(hostBuilder));
            }

            // Check if `UseIISServer` was called already
            if (hostBuilder.GetSetting(nameof(UseIIS)) != null)
            {
                return hostBuilder;
            }

            // Check if in process
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows) && NativeMethods.IsAspNetCoreModuleLoaded())
            {
                hostBuilder.UseSetting(nameof(UseIIS), "true");
                hostBuilder.CaptureStartupErrors(true);

                var iisConfigData = NativeMethods.HttpGetApplicationProperties();
                hostBuilder.UseContentRoot(iisConfigData.pwzFullApplicationPath);
                return hostBuilder.ConfigureServices(
                    services => {
                        foreach (var service in services)
                        {
                            // TODO: Workaround for removing startup filter from IISIntegration
                            if (service.ImplementationInstance?.GetType().Name == "IISServerSetupFilter")
                            {
                                services.Remove(service);
                                break;
                            }
                        }

                        services.AddSingleton<IServer, IISHttpServer>();
                        services.AddSingleton<IStartupFilter>(new IISServerSetupFilter(iisConfigData.pwzVirtualApplicationPath));
                        services.AddAuthenticationCore();
                        services.Configure<IISServerOptions>(
                             options => { options.ForwardWindowsAuthentication = iisConfigData.fWindowsAuthEnabled || iisConfigData.fBasicAuthEnabled; }
                        );
                    });
            }

            return hostBuilder;
        }
    }
}