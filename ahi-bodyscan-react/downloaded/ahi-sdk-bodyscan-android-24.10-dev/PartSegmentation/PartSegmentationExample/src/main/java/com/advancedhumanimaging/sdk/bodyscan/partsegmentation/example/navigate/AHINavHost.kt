//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.navigate

import androidx.compose.runtime.Composable
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.view.SegmentationView
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.view.SelectImagesView
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.view.SelectJointsView
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.viewmodel.AHIBSSegmentationViewModel

object NavRoute {
    const val HOME_SCREEN = "home"
    const val RESULT_SCREEN = "result"
    const val JOINTS_SCREEN = "joints"
}

@Composable
fun AHINavHost(navHostController: NavHostController, viewModel: AHIBSSegmentationViewModel) {
    NavHost(navController = navHostController, startDestination = NavRoute.HOME_SCREEN) {
        composable(NavRoute.HOME_SCREEN) {
            SelectImagesView(navHostController, viewModel)
        }
        composable(NavRoute.RESULT_SCREEN) {
            SegmentationView(navHostController, viewModel)
        }
        composable(NavRoute.JOINTS_SCREEN) {
            SelectJointsView(navHostController, viewModel)
        }
    }
}