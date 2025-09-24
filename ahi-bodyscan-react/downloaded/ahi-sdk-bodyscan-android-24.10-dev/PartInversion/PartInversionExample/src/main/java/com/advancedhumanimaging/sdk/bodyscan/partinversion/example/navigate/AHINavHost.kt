//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partinversion.example.navigate

import androidx.compose.runtime.Composable
import androidx.fragment.app.FragmentManager
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.view.InversionInputView
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.view.InversionResultView
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.viewmodel.AHIBSInversionViewModel

object NavRoute {
    const val INPUT_SCREEN = "input"
    const val RESULT_SCREEN = "result"
}

@Composable
fun AHINavHost(
    navHostController: NavHostController,
    viewModel: AHIBSInversionViewModel,
    fragmentManager: FragmentManager
) {
    NavHost(navController = navHostController, startDestination = NavRoute.INPUT_SCREEN) {
        composable(NavRoute.INPUT_SCREEN) {
            InversionInputView(navHostController, viewModel)
        }
        composable(NavRoute.RESULT_SCREEN) {
            InversionResultView(navHostController, viewModel, fragmentManager)
        }
    }
}