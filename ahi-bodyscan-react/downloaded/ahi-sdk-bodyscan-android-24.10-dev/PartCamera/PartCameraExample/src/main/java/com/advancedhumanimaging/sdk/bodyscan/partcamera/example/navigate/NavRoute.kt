//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcamera.example.navigate

import androidx.compose.runtime.Composable
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import com.advancedhumanimaging.sdk.bodyscan.partcamera.example.view.CameraCaptureView
import com.advancedhumanimaging.sdk.bodyscan.partcamera.example.view.GalleryPreview
import com.advancedhumanimaging.sdk.bodyscan.partcamera.example.viewmodel.AHICameraViewModel

object NavRoute {
    const val CAMERA_SCREEN = "camera"
    const val REVIEW_SCREEN = "review"
}

@Composable
fun AHINavHost(
    navHostController: NavHostController,
    viewModel: AHICameraViewModel,
) {
    NavHost(navController = navHostController, startDestination = NavRoute.CAMERA_SCREEN) {
        composable(NavRoute.CAMERA_SCREEN){
            CameraCaptureView(navHostController = navHostController, viewModel = viewModel)
        }
        composable(NavRoute.REVIEW_SCREEN){
            GalleryPreview(navHostController = navHostController, viewModel = viewModel)
        }
    }
}