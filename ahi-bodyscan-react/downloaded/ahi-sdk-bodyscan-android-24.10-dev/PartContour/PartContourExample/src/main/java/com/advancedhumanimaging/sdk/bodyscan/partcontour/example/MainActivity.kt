//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcontour.example

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.Modifier
import androidx.navigation.NavType
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.navArgument
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partcontour.example.ui.screens.*
import com.advancedhumanimaging.sdk.bodyscan.partcontour.example.ui.theme.AppTheme
import com.advancedhumanimaging.sdk.bodyscan.partcontour.example.viewmodel.ContourViewModel


class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val navController = rememberNavController()
            val contourViewModel = ContourViewModel(assets)
            AppTheme {
                NavHost(
                    modifier = Modifier
                        .fillMaxSize()
                        .background(MaterialTheme.colorScheme.background),
                    navController = navController,
                    startDestination = "home"
                ) {
                    composable("home") { Home(navController, contourViewModel) }
                    composable("idealContourMask") { IdealContourMask(navController, contourViewModel) }
                    composable("scaledContour") { ScaledContour(navController, contourViewModel) }
                    composable("optimalZones") { OptimalZones(navController, contourViewModel) }
                }
            }
        }
    }

    companion object {
        const val TAG = "ContourGenerator"
    }
}