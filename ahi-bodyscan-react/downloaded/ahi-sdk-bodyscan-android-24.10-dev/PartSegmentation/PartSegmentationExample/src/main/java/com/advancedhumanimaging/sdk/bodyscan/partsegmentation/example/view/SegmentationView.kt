//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.view

import android.graphics.Bitmap
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.Segmentation
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.ui.theme.BodyscanTheme
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.viewmodel.AHIBSSegmentationViewModel
import kotlinx.coroutines.launch

@Composable
@OptIn(ExperimentalMaterial3Api::class)
fun SegmentationView(
    naviHostController: NavHostController,
    viewModel: AHIBSSegmentationViewModel,
) {
    var showDialog by remember { mutableStateOf(false) }
    val context = LocalContext.current
    var resultBitmap by remember { mutableStateOf(Bitmap.createBitmap(1, 1, Bitmap.Config.ARGB_8888)) }
    BodyscanTheme {
        Image(
            bitmap = resultBitmap.asImageBitmap(),
            contentDescription = "",
            alignment = Alignment.Center,
            contentScale = ContentScale.FillHeight,
            modifier = Modifier.fillMaxHeight()
        )
    }

    if (showDialog) {
        Dialog(
            onDismissRequest = { showDialog = false },
            DialogProperties(dismissOnBackPress = false, dismissOnClickOutside = false)
        ) {
            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier
                    .padding(16.dp)
                    .background(Color.White, shape = RoundedCornerShape(8.dp))
            ) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Center
                ) {
                    CircularProgressIndicator(modifier = Modifier.padding(4.dp))
                    Text(modifier = Modifier.padding(4.dp), text = "Segmenting, please wait...")
                }
            }
        }
    }

    LaunchedEffect(true) {
        showDialog = true
        val result =
            Segmentation.segment(
                viewModel.humanBitmap,
                viewModel.contourBitmap,
                viewModel.profile,
                viewModel.poseJoints,
                context,
                Resources()
            ).getOrNull()
        result?.let { resultBitmap = it }
        showDialog = false
    }
}

