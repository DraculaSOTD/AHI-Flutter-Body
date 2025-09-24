package com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.navigate

import androidx.compose.runtime.Composable
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.view.AHIBSPoseDetectionView
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.view.AHIBSPoseDetectionResultView
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.viewmodel.AHIBSPoseDetectionViewModel

object NavRoute {
    const val HOME_SCREEN = "home"
    const val RESULT_SCREEN = "result"
}


@Composable
fun AHINavHost(navHostController: NavHostController, viewModel: AHIBSPoseDetectionViewModel) {
    NavHost(navController = navHostController, startDestination = NavRoute.HOME_SCREEN) {
        composable(NavRoute.HOME_SCREEN) {
            AHIBSPoseDetectionView(navHostController, viewModel)
        }

        composable(NavRoute.RESULT_SCREEN) {
            AHIBSPoseDetectionResultView(navHostController, viewModel)
        }
    }
}

