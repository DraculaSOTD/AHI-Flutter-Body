//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example.view

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.advancedhumanimaging.sdk.bodyscan.example.MainScreenState
import com.advancedhumanimaging.sdk.bodyscan.example.MainViewModel
import com.advancedhumanimaging.sdk.bodyscan.example.SdkOperationStatus
import com.advancedhumanimaging.sdk.bodyscan.example.components.AppTitle
import com.advancedhumanimaging.sdk.bodyscan.example.components.PrimaryButton
import com.advancedhumanimaging.sdk.common.IAHIDownloadProgress
import com.advancedhumanimaging.sdk.common.models.AHIResult
import com.advancedhumanimaging.sdk.multiscan.AHIMultiScan

@Composable
fun SdkSetupCard(
    uiState: MainScreenState,
    mainViewModel: MainViewModel
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(8.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(top = 16.dp, bottom = 16.dp, start = 4.dp, end = 4.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            AppTitle(label = "SDK SETUP")
            Spacer(modifier = Modifier.height(16.dp))
            SdkSetupUI(uiState.sdkSetupState, mainViewModel, false)
            //Hiding custom UI setup option for now as it is not functional at the moment
            //Spacer(modifier = Modifier.height(16.dp))
            //SdkSetupUI(uiState.sdkSetupState, mainViewModel, true)
            SdkAuthorizationUI(uiState = uiState, mainViewModel = mainViewModel)
            SdkDownloadResourcesUI(uiState, mainViewModel)
            ReleaseSdkUI(uiState = uiState, mainViewModel = mainViewModel)
        }
    }
}

@Composable
fun SdkSetupUI(
    setupStatus: SdkOperationStatus,
    mainViewModel: MainViewModel,
    customUI: Boolean,
) {
    when (setupStatus) {
        SdkOperationStatus.SUCCESS -> Text(text = "SDK Initialised")
        SdkOperationStatus.NOT_DONE -> {
            PrimaryButton(
                onPress = { mainViewModel.triggerSdkInit(customUI) },
                label = "Setup MultiScan SDK ${if (customUI) " - Custom UI" else ""}"
            )
        }
        SdkOperationStatus.IN_PROGRESS -> {
            CircularProgressIndicator()
        }
    }
}

@Composable
fun SdkAuthorizationUI(
    uiState: MainScreenState,
    mainViewModel: MainViewModel,
) {
    if (uiState.sdkSetupState != SdkOperationStatus.SUCCESS) {
        return
    }
    Spacer(modifier = Modifier.height(16.dp))
    when (uiState.sdkAuthZState) {
        SdkOperationStatus.SUCCESS -> Text(text = "User Authorized")
        SdkOperationStatus.NOT_DONE -> {
            PrimaryButton(
                onPress = { mainViewModel.triggerUserAuthZ() },
                label = "Authorize User"
            )
        }
        SdkOperationStatus.IN_PROGRESS -> {
            CircularProgressIndicator()
        }
    }
}

@Composable
fun ReleaseSdkUI(
    uiState: MainScreenState,
    mainViewModel: MainViewModel,
) {
    if (uiState.sdkSetupState != SdkOperationStatus.SUCCESS) {
        return
    }
    Spacer(modifier = Modifier.height(16.dp))
    when (uiState.sdkReleaseState) {
        SdkOperationStatus.NOT_DONE -> {
            PrimaryButton(
                onPress = { mainViewModel.triggerSdkRelease() },
                label = "Release SDK"
            )
        }
        SdkOperationStatus.IN_PROGRESS -> {
            CircularProgressIndicator()
        }
        SdkOperationStatus.SUCCESS -> {
            // show nothing
        }
    }
}

@Composable
fun SdkDownloadResourcesUI(
    uiState: MainScreenState,
    mainViewModel: MainViewModel,
) {
    if (uiState.sdkSetupState != SdkOperationStatus.SUCCESS || uiState.sdkAuthZState != SdkOperationStatus.SUCCESS) {
        return
    }
    Spacer(modifier = Modifier.height(16.dp))
    when (uiState.sdkResourceDownloadState) {
        SdkOperationStatus.SUCCESS -> Text(text = "Resources downloaded")
        SdkOperationStatus.NOT_DONE -> {
            PrimaryButton(
                onPress = {
                    AHIMultiScan.delegateDownloadProgress = object : IAHIDownloadProgress {
                        override fun downloadProgressReport(status: AHIResult<Unit>) {
                            mainViewModel.updateDownloadResourcesState(status)
                        }
                    }
                    mainViewModel.downloadResources()
                },
                label = "Download Resources"
            )
        }
        SdkOperationStatus.IN_PROGRESS -> {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.Center,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(text = uiState.resultMessage)
                Spacer(modifier = Modifier.width(8.dp))
                CircularProgressIndicator()
            }
        }
    }
}