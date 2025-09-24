//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partclassification.example.navigate

import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.navigation.NavHostController
import androidx.navigation.NavType
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.navArgument
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.ui.screen.AHIBSClassificationHome
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.ui.screen.AHIBSClassificationPickSilhouette
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.ui.screen.AHIBSClassificationResult
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.viewmodel.AHIBSClassificationViewModel

object NavRoutes {
    const val HOME_SCREEN = "home"
    const val RESULT_SCREEN = "result"
    const val PICK_SILHOUETTE_SCREEN = "pickSilhouette"
}

@Composable
fun AHINaviHost(
    modifier: Modifier = Modifier,
    navHostController: NavHostController,
    viewModel: AHIBSClassificationViewModel
) {
    NavHost(navController = navHostController, startDestination = NavRoutes.HOME_SCREEN) {
        composable(NavRoutes.HOME_SCREEN) {
            AHIBSClassificationHome(
                modifier = modifier,
                navHostController = navHostController,
                viewModel = viewModel
            )
        }
        composable(NavRoutes.RESULT_SCREEN) {
            AHIBSClassificationResult(
                modifier = modifier,
                navHostController = navHostController,
                viewModel = viewModel
            )
        }
        composable(
            "${NavRoutes.PICK_SILHOUETTE_SCREEN}/{profile}",
            arguments = listOf(
                navArgument("profile") { type = NavType.StringType }
            )
        ) { backStackEntry ->
            AHIBSClassificationPickSilhouette(
                modifier = modifier,
                navHostController = navHostController,
                viewModel = viewModel,
                backStackEntry.arguments?.getString("profile")?.let { it -> Profile.valueOf(it) } ?: Profile.front
            )
        }
    }
}