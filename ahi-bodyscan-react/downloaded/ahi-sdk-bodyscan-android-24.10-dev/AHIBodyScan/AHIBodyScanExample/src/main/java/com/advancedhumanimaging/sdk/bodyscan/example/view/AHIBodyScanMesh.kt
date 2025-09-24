//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.example.view

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.fragment.app.FragmentManager
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.example.MainScreenState
import com.advancedhumanimaging.sdk.bodyscan.example.MainViewModel
import com.advancedhumanimaging.sdk.bodyscan.example.SdkOperationStatus
import com.advancedhumanimaging.sdk.bodyscan.example.fragment.BodyScanModelFragment
import com.advancedhumanimaging.sdk.bodyscan.example.fragment.FragmentContainer

@Composable
@OptIn(ExperimentalMaterial3Api::class)
fun AHIBodyScanMesh(
    navHostController: NavHostController,
    mainViewModel: MainViewModel,
    modifier: Modifier = Modifier,
    fragmentManager: FragmentManager
) {
    val uiState by mainViewModel.getMainScreenState()
        .observeAsState(initial = MainScreenState())
    Scaffold(
        modifier = modifier,
        topBar = {
            CenterAlignedTopAppBar(
                title = { Text("BodyScan Mesh") },
                navigationIcon = {
                    IconButton(onClick = {
                        navHostController.popBackStack()
                    }) {
                        Icon(
                            imageVector = Icons.Filled.ArrowBack,
                            contentDescription = "Localized description"
                        )
                    }
                }
            )
        }
    ) { padding ->
        Box(modifier = modifier.padding(padding)) {
            when (uiState.meshGenerationState) {
                SdkOperationStatus.IN_PROGRESS -> {
                    CircularProgressIndicator(modifier = Modifier.align(Alignment.Center))
                }
                SdkOperationStatus.SUCCESS -> {
                    val meshUri = uiState.meshLocation
                    val height = uiState.classificationResults["cm_ent_height"].toString().toFloatOrNull()
                    if (meshUri != null && height != null) {
                        FragmentContainer(
                            modifier = Modifier
                                .fillMaxSize(),
                            fragmentManager = fragmentManager,
                            commit = {
                                add(
                                    it,
                                    BodyScanModelFragment(meshUri, height)
                                )
                            }
                        )
                    } else {
                        Text(
                            modifier = Modifier.align(Alignment.Center),
                            text = "Mesh generation results are empty"
                        )
                    }
                }
                SdkOperationStatus.NOT_DONE -> {
                    Text(
                        modifier = Modifier.align(Alignment.Center),
                        text = mainViewModel.getMainScreenState().value?.resultMessage ?: ""
                    )
                }
            }
        }
    }
}