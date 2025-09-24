//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example.view

import android.Manifest
import androidx.activity.result.ActivityResultRegistry
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.layout.layoutId
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.constraintlayout.compose.ConstraintLayout
import androidx.constraintlayout.compose.ConstraintSet
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.example.MainScreenState
import com.advancedhumanimaging.sdk.bodyscan.example.MainViewModel
import com.advancedhumanimaging.sdk.bodyscan.example.navigate.NavRoutes
import com.google.accompanist.permissions.ExperimentalPermissionsApi
import com.google.accompanist.permissions.PermissionStatus
import com.google.accompanist.permissions.rememberPermissionState
import com.google.accompanist.permissions.shouldShowRationale

@OptIn(ExperimentalPermissionsApi::class, ExperimentalMaterial3Api::class)
@Composable
fun AHIBodyScanHome(
    navHostController: NavHostController,
    mainViewModel: MainViewModel,
    activityResultRegistry: ActivityResultRegistry,
    modifier: Modifier = Modifier
) {
    Scaffold(
        modifier = modifier
    ) { contentPadding ->
        Box(modifier = modifier.padding(contentPadding)) {
            val cameraPermissionState = rememberPermissionState(permission = Manifest.permission.CAMERA)
            if (cameraPermissionState.status == PermissionStatus.Granted) {
                val uiState by mainViewModel.getMainScreenState()
                    .observeAsState(initial = MainScreenState())
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(top = 8.dp, bottom = 8.dp),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    SdkSetupCard(uiState, mainViewModel)
                    ScanCard(uiState, mainViewModel, navHostController, activityResultRegistry)
                }
            } else {
                Column(
                    modifier = modifier.fillMaxWidth(),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Center
                ) {
                    val textShow =
                        if (cameraPermissionState.status.shouldShowRationale) {
                            "The camera is important for this app.\nPlease grant the permission."
                        } else {
                            "Camera permission required for this feature to be available.\nPlease grant the permission."
                        }
                    Text(text = textShow, textAlign = TextAlign.Center)
                    Spacer(modifier = Modifier.height(32.dp))
                    Button(onClick = { cameraPermissionState.launchPermissionRequest() }) {
                        Text(text = "Request permission")
                    }
                }
            }
        }
        BoxWithConstraints {
            ConstraintLayout(modifier = Modifier.fillMaxSize(), constraintSet = constraints()) {
                ExtendedFloatingActionButton(
                    modifier = Modifier.layoutId("settings"),
                    text = { Text(text = "THEME") },
                    icon = { Icon(Icons.Filled.Settings, contentDescription = null) },
                    onClick = { navHostController.navigate(NavRoutes.THEME_SCREEN) }
                )
            }
        }
    }
}

private fun constraints(): ConstraintSet {
    return ConstraintSet {
        val settings = createRefFor("settings")
        constrain(settings) {
            bottom.linkTo(parent.bottom, margin = 8.dp)
            end.linkTo(parent.end, margin = 8.dp)
        }
    }
}