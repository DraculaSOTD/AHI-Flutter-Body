//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example

import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.activity.viewModels
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.fragment.app.FragmentActivity
import androidx.navigation.compose.rememberNavController
import com.advancedhumanimaging.sdk.bodyscan.example.navigate.AHINavHost
import com.advancedhumanimaging.sdk.bodyscan.example.ui.theme.BodyScanExampleAppTheme

class MainActivity : FragmentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val mainViewModel by viewModels<MainViewModel>()
        setContent {
            BodyScanExampleAppTheme {
                Surface(color = MaterialTheme.colorScheme.background) {
                    val navHostController = rememberNavController()
                    AHINavHost(navHostController = navHostController, viewModel = mainViewModel, activityResultRegistry, supportFragmentManager)
                }
            }
        }
    }
}