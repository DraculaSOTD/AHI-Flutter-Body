//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example.navigate

import androidx.activity.result.ActivityResultRegistry
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.fragment.app.FragmentManager
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import com.advancedhumanimaging.sdk.bodyscan.example.MainViewModel
import com.advancedhumanimaging.sdk.bodyscan.example.view.AHIBodyScanHome
import com.advancedhumanimaging.sdk.bodyscan.example.view.AHIBodyScanMesh
import com.advancedhumanimaging.sdk.bodyscan.example.view.AHIBodyScanResult
import com.advancedhumanimaging.sdk.bodyscan.example.view.AHIBodyScanTheme

object NavRoutes {
    const val HOME_SCREEN = "home"
    const val RESULT_SCREEN = "result"
    const val MESH_SCREEN = "mesh"
    const val THEME_SCREEN = "theme"
}

@Composable
fun AHINavHost(
    navHostController: NavHostController,
    viewModel: MainViewModel,
    activityResultRegistry: ActivityResultRegistry,
    fragmentManager: FragmentManager,
) {
    val modifier = Modifier
        .fillMaxSize()
        .padding(8.dp)
    NavHost(navController = navHostController, startDestination = NavRoutes.HOME_SCREEN) {
        composable(NavRoutes.HOME_SCREEN) {
            AHIBodyScanHome(
                navHostController = navHostController,
                mainViewModel = viewModel,
                activityResultRegistry = activityResultRegistry,
                modifier = modifier
            )
        }
        composable(NavRoutes.RESULT_SCREEN) {
            AHIBodyScanResult(navHostController = navHostController, mainViewModel = viewModel, modifier = modifier)
        }
        composable(NavRoutes.MESH_SCREEN) {
            AHIBodyScanMesh(navHostController = navHostController, mainViewModel = viewModel, modifier = modifier, fragmentManager)
        }
        composable(NavRoutes.THEME_SCREEN) {
            AHIBodyScanTheme(navHostController = navHostController, mainViewModel = viewModel, modifier = modifier)
        }

    }
}