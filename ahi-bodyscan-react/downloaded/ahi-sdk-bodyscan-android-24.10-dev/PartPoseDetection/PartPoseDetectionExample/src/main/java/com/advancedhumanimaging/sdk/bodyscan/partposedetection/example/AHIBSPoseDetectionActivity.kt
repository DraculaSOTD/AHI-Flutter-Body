//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposedetection.example

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.compose.rememberNavController
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.navigate.AHINavHost
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.viewmodel.AHIBSPoseDetectionViewModel

class AHIBSPoseDetectionActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val viewModel = ViewModelProvider(this).get(AHIBSPoseDetectionViewModel::class.java)
            val navHostController = rememberNavController()
            AHINavHost(navHostController = navHostController, viewModel)
        }
    }
}
