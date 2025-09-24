//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partresources.example

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import com.advancedhumanimaging.sdk.bodyscan.partresources.example.components.AppTitle
import com.advancedhumanimaging.sdk.bodyscan.partresources.example.components.PrimaryButton
import com.advancedhumanimaging.sdk.common.IAHIDownloadProgress
import com.advancedhumanimaging.sdk.common.models.AHIResult
import com.advancedhumanimaging.sdk.multiscan.AHIMultiScan

class MainActivity : ComponentActivity() {
    @OptIn(ExperimentalMaterial3Api::class)
    override fun onCreate(savedInstanceState: Bundle?) {
        val mainViewModel by viewModels<MainViewModel>()
        super.onCreate(savedInstanceState)

        setContent {
            Scaffold(
                modifier = Modifier,
                topBar = {
                    CenterAlignedTopAppBar(
                        title = { Text("RESOURCES") }
                    )
                }
            ) { contentPadding ->
                val uiState by mainViewModel.getMainScreenState().observeAsState(initial = MainScreenState())
                val scroll = rememberScrollState()
                Column(
                    modifier = Modifier
                        .padding(contentPadding)
                        .fillMaxWidth()
                        .verticalScroll(scroll), horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    SdkSetupUI(uiState, mainViewModel = mainViewModel)
                    SdkDownloadResourcesUI(uiState, mainViewModel)
                    DecryptResourcesUI(uiState, mainViewModel)
                }
            }
        }
    }
}

@Composable
fun SdkSetupUI(
    uiState: MainScreenState,
    mainViewModel: MainViewModel,
) {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(8.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            AppTitle(label = "SDK SETUP")
            Spacer(modifier = Modifier.height(16.dp))
            when (uiState.sdkSetupState) {
                SdkOperationStatus.SUCCESS -> Text(text = "SDK Initialised")
                SdkOperationStatus.NOT_DONE -> {
                    PrimaryButton(
                        onPress = { mainViewModel.triggerSdkInit() }, label = "Setup MultiScan SDK"
                    )
                }
                SdkOperationStatus.IN_PROGRESS -> {
                    CircularProgressIndicator()
                }
            }
            ReleaseSdkUI(uiState, mainViewModel)
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
                onPress = { mainViewModel.triggerSdkRelease() }, label = "Release SDK"
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
    if (uiState.sdkSetupState != SdkOperationStatus.SUCCESS) {
        return
    }
    var showListDialog by remember { mutableStateOf(false) }
    val filesConfigList = AHIMultiScan.getFilesList().getOrNull()?.get("1")?.map { it["name_ext"] as? String }
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(8.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            AppTitle(label = "RESOURCE DOWNLOAD")
            Spacer(modifier = Modifier.height(16.dp))
            PrimaryButton(
                onPress = {
                    showListDialog = true
                }, label = "List ${filesConfigList?.size ?: 0} Resources"
            )
            Spacer(modifier = Modifier.height(16.dp))
            if (showListDialog) {
                Dialog(
                    onDismissRequest = { showListDialog = false },
                    DialogProperties(dismissOnBackPress = true, dismissOnClickOutside = true)
                ) {
                    if (filesConfigList != null) {
                        Card(
                            modifier = Modifier
                        ) {
                            LazyColumn(
                                modifier = Modifier
                                    .padding(8.dp),
                                horizontalAlignment = Alignment.Start,
                            ) {
                                itemsIndexed(filesConfigList) { index, item ->
                                    Text(
                                        modifier = Modifier.padding(4.dp),
                                        text = "$item"
                                    )
                                    if (index < filesConfigList.lastIndex)
                                        Divider(color = Color.Black, thickness = 1.dp)
                                }
                            }
                        }

                    } else {
                        Text(text = "Cannot get files config list")
                    }
                }
            }
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
                        }, label = "Download Resources"
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
            SdkRemoveDownloadResourcesUI(uiState, mainViewModel)
        }
    }
}

@Composable
fun SdkRemoveDownloadResourcesUI(
    uiState: MainScreenState, mainViewModel: MainViewModel
) {
    if (uiState.sdkSetupState != SdkOperationStatus.SUCCESS || uiState.sdkResourceDownloadState != SdkOperationStatus.SUCCESS) {
        return
    }
    Spacer(modifier = Modifier.height(16.dp))
    when (uiState.sdkRemoveDownloadResourceState) {
        SdkOperationStatus.SUCCESS -> {
            /*Do nothing*/
        }
        SdkOperationStatus.NOT_DONE -> {
            PrimaryButton(
                onPress = {
                    mainViewModel.triggerReleaseDownloadResourceState()
                }, label = "Clear Download Resources"
            )
        }
        SdkOperationStatus.IN_PROGRESS -> {
            Column(
                modifier = Modifier.fillMaxWidth(), horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Spacer(modifier = Modifier.width(8.dp))
                CircularProgressIndicator()
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DecryptResourcesUI(
    uiState: MainScreenState, mainViewModel: MainViewModel
) {
    if (uiState.sdkSetupState != SdkOperationStatus.SUCCESS || uiState.sdkResourceDownloadState != SdkOperationStatus.SUCCESS) {
        return
    }
    val resources =
        LocalContext.current.filesDir.listFiles()?.filter { it.path.endsWith(".bin") }?.map { it.name }?.sorted() ?: listOf()
    var expanded by remember { mutableStateOf(false) }
    var selectedOptionText by remember { mutableStateOf(resources.firstOrNull() ?: "") }
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(8.dp)
    ) {
        Column(
            Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            AppTitle(label = "RESOURCE DECRYPTION")
            Spacer(modifier = Modifier.height(16.dp))
            ExposedDropdownMenuBox(
                expanded = expanded,
                onExpandedChange = {
                    expanded = !expanded
                }
            ) {
                OutlinedTextField(
                    modifier = Modifier.menuAnchor(),
                    readOnly = true,
                    value = selectedOptionText,
                    onValueChange = { },
                    label = { Text("Resource") },
                    trailingIcon = {
                        ExposedDropdownMenuDefaults.TrailingIcon(
                            expanded = expanded
                        )
                    },
                    colors = ExposedDropdownMenuDefaults.textFieldColors()
                )
                ExposedDropdownMenu(
                    expanded = expanded,
                    onDismissRequest = {
                        expanded = false
                    }
                ) {
                    resources.forEach { selectionOption ->
                        DropdownMenuItem(
                            onClick = {
                                selectedOptionText = selectionOption
                                expanded = false
                                mainViewModel.resetResultMessage()
                            },
                            text = { Text(selectionOption) }
                        )
                    }
                }
            }
            Spacer(modifier = Modifier.height(16.dp))
            PrimaryButton(
                onPress = {
                    mainViewModel.decryptResource(selectedOptionText)
                }, label = "Decrypt Resources"
            )
            when (uiState.decryptResourceState) {
                SdkOperationStatus.IN_PROGRESS -> {
                    Spacer(modifier = Modifier.height(16.dp))
                    CircularProgressIndicator()
                }
                else -> {
                    if (uiState.resultMessage.isNotBlank())
                        Spacer(modifier = Modifier.height(16.dp))
                    Text(uiState.resultMessage)
                }
            }
        }
    }
}