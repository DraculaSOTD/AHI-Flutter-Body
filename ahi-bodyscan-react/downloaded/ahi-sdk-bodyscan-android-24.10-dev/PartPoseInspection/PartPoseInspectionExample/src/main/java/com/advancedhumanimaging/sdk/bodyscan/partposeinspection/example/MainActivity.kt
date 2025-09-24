package com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.ui.Modifier
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example.ui.screens.ContourMask
import com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example.ui.screens.PoseInspection
import com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example.ui.theme.AppTheme
import com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example.viewmodel.InspectionViewModel

class MainActivity : ComponentActivity() {
    @OptIn(ExperimentalMaterial3Api::class)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val navController = rememberNavController()
            val contourViewModel = InspectionViewModel(assets)
            AppTheme {
                NavHost(
                    modifier = Modifier
                        .fillMaxSize()
                        .background(MaterialTheme.colorScheme.background),
                    navController = navController,
                    startDestination = "contourMask"
                ) {
                    composable("contourMask") { ContourMask(navController, contourViewModel) }
                    composable("poseInspection") { PoseInspection(navController, contourViewModel) }
                }
            }
        }
    }

    companion object {
        const val TAG = "MainActivity"
    }

}