//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partclassification.example

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.Modifier
import androidx.navigation.compose.rememberNavController
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.navigate.AHINaviHost
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.ui.theme.AppTheme
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.viewmodel.AHIBSClassificationViewModel

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            val viewModel = AHIBSClassificationViewModel(assets)
            val navHostController = rememberNavController()
            AppTheme {
                AHINaviHost(
                    modifier = Modifier
                        .fillMaxSize()
                        .background(MaterialTheme.colorScheme.background),
                    navHostController = navHostController,
                    viewModel = viewModel
                )
            }
        }
    }
}