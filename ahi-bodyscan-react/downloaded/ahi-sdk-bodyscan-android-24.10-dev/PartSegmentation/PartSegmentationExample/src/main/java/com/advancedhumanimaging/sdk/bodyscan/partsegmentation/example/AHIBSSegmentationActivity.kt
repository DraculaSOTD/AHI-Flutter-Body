//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.compose.rememberNavController
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.navigate.AHINavHost
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.viewmodel.AHIBSSegmentationViewModel

class AHIBSSegmentationActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val viewModel = ViewModelProvider(this).get(AHIBSSegmentationViewModel::class.java)
            val navHostController = rememberNavController()
            AHINavHost(navHostController = navHostController, viewModel)
        }
    }
}