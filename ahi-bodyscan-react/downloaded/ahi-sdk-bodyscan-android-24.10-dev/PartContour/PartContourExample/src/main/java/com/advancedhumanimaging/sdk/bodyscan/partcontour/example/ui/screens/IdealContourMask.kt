//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcontour.example.ui.screens

import android.annotation.SuppressLint
import android.graphics.Bitmap
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.scale
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.partcontour.example.viewmodel.ContourViewModel

private const val TAG = "IdealContour"

@SuppressLint("CoroutineCreationDuringComposition")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun IdealContourMask(navController: NavController, viewModel: ContourViewModel) {
    var imageSize by remember { mutableStateOf(IntSize.Zero) }
    val painter = object : Painter() {
        val scale = imageSize.width / AHIBSImageCaptureWidth
        override val intrinsicSize: Size
            get() = Size(scale * AHIBSImageCaptureWidth, scale * AHIBSImageCaptureHeight)

        override fun DrawScope.onDraw() {
            val bitmap =
                if (viewModel.profile == Profile.front) viewModel.contourMaskFront else viewModel.contourMaskSide
            bitmap?.asImageBitmap()?.let {
                scale(
                    scaleX = scale,
                    scaleY = scale,
                    Offset(0F, 0F)
                ) {
                    drawImage(it)
                }
            }
        }
    }
    Scaffold(
        modifier = Modifier
            .fillMaxSize()
            .padding(8.dp),
        topBar = {
            CenterAlignedTopAppBar(
                title = { Text("Ideal Contour Mask") },
                navigationIcon = {
                    IconButton(onClick = { navController.popBackStack() }) {
                        Icon(
                            imageVector = Icons.Filled.ArrowBack,
                            contentDescription = "Localized description"
                        )
                    }
                }
            )
        }
    ) { contentPadding ->
        Column(
            modifier = Modifier
                .padding(contentPadding)
                .fillMaxSize(),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(8.dp)
                    .onSizeChanged {
                        imageSize = it
                    },
                contentAlignment = Alignment.Center
            ) {
                Card(
                    colors = CardDefaults.cardColors(
                        containerColor = Color.Transparent,
                        contentColor = Color.Transparent
                    )
                ) {
                    Image(
                        modifier = Modifier
                            .background(Color.Black),
                        painter = painter,
                        contentDescription = "",
                        alignment = Alignment.Center,
                        contentScale = ContentScale.Fit,
                    )
                }
            }
        }
    }
}