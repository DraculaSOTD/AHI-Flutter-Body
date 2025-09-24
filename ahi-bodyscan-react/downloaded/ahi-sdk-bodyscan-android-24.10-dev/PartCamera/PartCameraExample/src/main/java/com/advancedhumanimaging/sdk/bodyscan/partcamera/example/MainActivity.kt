//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcamera.example

import android.Manifest
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.material.Button
import androidx.compose.material.Text
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.compose.rememberNavController
import com.advancedhumanimaging.sdk.bodyscan.partcamera.example.navigate.AHINavHost
import com.advancedhumanimaging.sdk.bodyscan.partcamera.example.viewmodel.AHICameraViewModel
import com.google.accompanist.permissions.*

class MainActivity : ComponentActivity() {
    @OptIn(ExperimentalPermissionsApi::class)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val multiplePermissionState = rememberMultiplePermissionsState(
                permissions = listOf(
                    Manifest.permission.CAMERA,
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE
                )
            )
            PermissionsRequired(
                multiplePermissionsState = multiplePermissionState,
                permissionsNotGrantedContent = {
                    Column(modifier = Modifier.fillMaxSize()) {
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(bottom = 16.dp),
                            horizontalArrangement = Arrangement.Center,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Button(onClick = {
                                multiplePermissionState.launchMultiplePermissionRequest()
                            }) {
                                Text(text = "Grant Permission")
                            }
                        }
                    }
                },
                permissionsNotAvailableContent = {
                    Column(modifier = Modifier.fillMaxSize()) {
                        Row(
                            modifier = Modifier
                                .fillMaxWidth()
                                .padding(bottom = 16.dp),
                            horizontalArrangement = Arrangement.Center,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Button(onClick = {
                                multiplePermissionState.launchMultiplePermissionRequest()
                            }) {
                                Text(text = "Grant Permission")
                            }
                        }
                    }
                }
            ) {
                val viewModel = ViewModelProvider(this).get(AHICameraViewModel::class.java)
                val navHostController = rememberNavController()
                AHINavHost(navHostController = navHostController, viewModel = viewModel)
            }
        }
    }
}