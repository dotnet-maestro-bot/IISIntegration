// Copyright (c) .NET Foundation. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using IISIntegration.FunctionalTests.Utilities;
using Microsoft.AspNetCore.Server.IntegrationTesting;
using Microsoft.Extensions.Logging;
using Xunit;

namespace Microsoft.AspNetCore.Server.IISIntegration.FunctionalTests
{
    public class LoggingTests : IISFunctionalTestBase
    {
        [Theory]
        [InlineData("CheckErrLogFile")]
        [InlineData("CheckLogFile")]
        public async Task CheckStdoutLogging(string path)
        {
            var deploymentParameters = Helpers.GetBaseDeploymentParameters();
            deploymentParameters.PublishApplicationBeforeDeployment = true;
            deploymentParameters.PreservePublishedApplicationForDebugging = true; // workaround for keeping

            var deploymentResult = await DeployAsync(deploymentParameters);

            try
            {
                Helpers.ModifyAspNetCoreSectionInWebConfig(deploymentResult, "stdoutLogEnabled", "true");
                Helpers.ModifyAspNetCoreSectionInWebConfig(deploymentResult, "stdoutLogFile", @".\logs\stdout");

                var response = await deploymentResult.RetryingHttpClient.GetAsync(path);

                var responseText = await response.Content.ReadAsStringAsync();

                Assert.Equal("Hello World", responseText);

                StopServer();

                var folderPath = Path.Combine(deploymentResult.DeploymentResult.ContentRoot, @"logs");

                var fileInDirectory = Directory.GetFiles(folderPath).Single();
                Assert.NotNull(fileInDirectory);

                string contents = null;

                // RetryOperation doesn't support async lambdas, call synchronous ReadAllText.
                RetryHelper.RetryOperation(
                    () => contents = File.ReadAllText(fileInDirectory),
                    e => Logger.LogError($"Failed to read file: {e.Message}"),
                    retryCount: 10,
                    retryDelayMilliseconds: 100);

                Assert.NotNull(contents);
                Assert.Contains("TEST MESSAGE", contents);
            }
            finally
            {

                RetryHelper.RetryOperation(
                    () => Directory.Delete(deploymentParameters.PublishedApplicationRootPath, true),
                    e => Logger.LogWarning($"Failed to delete directory : {e.Message}"),
                    retryCount: 3,
                    retryDelayMilliseconds: 100);
            }
        }

        [Fact]
        public async Task StartupMessagesAreLoggedIntoDebugLogFile()
        {
            var tempFile = Path.GetTempFileName();
            try
            {
                var deploymentParameters = Helpers.GetBaseDeploymentParameters();
                deploymentParameters.EnvironmentVariables["ASPNETCORE_MODULE_DEBUG_FILE"] = tempFile;

                var deploymentResult = await DeployAsync(deploymentParameters);
                var response = await deploymentResult.RetryingHttpClient.GetAsync("/");

                StopServer();

                var logContents = File.ReadAllText(tempFile);
                Assert.Contains("[aspnetcorev2.dll]", logContents);
                Assert.Contains("[aspnetcorev2_inprocess.dll]", logContents);
            }
            finally
            {
                File.Delete(tempFile);
            }
        }

    }
}
