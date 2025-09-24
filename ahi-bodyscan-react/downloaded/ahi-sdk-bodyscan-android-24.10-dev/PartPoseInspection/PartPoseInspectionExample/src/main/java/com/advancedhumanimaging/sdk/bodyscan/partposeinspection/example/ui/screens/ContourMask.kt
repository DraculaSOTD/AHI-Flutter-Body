//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example.ui.screens

import android.annotation.SuppressLint
import android.graphics.Bitmap
import android.graphics.ImageDecoder
import android.os.Build
import android.provider.MediaStore
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.rememberSaveable
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
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example.viewmodel.InspectionViewModel

@SuppressLint("CoroutineCreationDuringComposition")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ContourMask(navController: NavController, inspectionViewModel: InspectionViewModel) {
    val context = LocalContext.current
    var sexInput by rememberSaveable { mutableStateOf(SexType.male) }
    var heightInput by rememberSaveable { mutableStateOf(190.0F) }
    var weightInput by rememberSaveable { mutableStateOf(70.0F) }
    var thetaAngleInput by rememberSaveable { mutableStateOf(0.0F) }
    var imageSize by remember { mutableStateOf(IntSize.Zero) }
    val humanImagePickerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent(),
        onResult = { imageUri ->
            imageUri?.let {
                val bitmap = if (Build.VERSION.SDK_INT < 28) {
                    MediaStore.Images.Media.getBitmap(navController.context.contentResolver, it).copy(Bitmap.Config.ARGB_8888, true)
                } else {
                    val source = ImageDecoder.createSource(navController.context.contentResolver, it)
                    ImageDecoder.decodeBitmap(source) { decoder, _, _ ->
                        decoder.isMutableRequired = true
                    }
                }
                inspectionViewModel.updateContourMask(Bitmap.createScaledBitmap(bitmap, 720, 1280, true))
            }
        }
    )
    val painter = object : Painter() {
        val scale = imageSize.height / AHIBSImageCaptureHeight
        override val intrinsicSize: Size
            get() = Size(scale * AHIBSImageCaptureWidth, scale * AHIBSImageCaptureHeight)

        override fun DrawScope.onDraw() {
            val bitmap = inspectionViewModel.contourMask
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
            .padding(4.dp),
        topBar = {
            CenterAlignedTopAppBar(
                title = { Text("Contour Mask") },
            )
        }
    ) { contentPadding ->
        Column(
            modifier = Modifier
                .padding(contentPadding)
                .fillMaxSize(),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Row(
                Modifier
                    .padding(4.dp)
                    .fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(text = "Sex:")
                Row(
                    Modifier
                        .selectableGroup()
                ) {
                    Row(
                        Modifier
                            .selectable(
                                selected = sexInput == SexType.male,
                                onClick = {
                                    sexInput = SexType.male
                                },
                                role = Role.RadioButton
                            )
                            .padding(4.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        RadioButton(
                            selected = sexInput == SexType.male,
                            onClick = null
                        )
                        Text(
                            text = "Male",
                            modifier = Modifier.padding(start = 8.dp)
                        )
                    }
                    Row(
                        Modifier
                            .selectable(
                                selected = sexInput == SexType.female,
                                onClick = {
                                    sexInput = SexType.female
                                },
                                role = Role.RadioButton
                            )
                            .padding(4.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        RadioButton(
                            selected = sexInput == SexType.female,
                            onClick = null
                        )
                        Text(
                            text = "Female",
                            modifier = Modifier.padding(start = 8.dp)
                        )
                    }
                }
            }
            Row(
                Modifier
                    .padding(4.dp)
                    .fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(text = "Profile:")
                Row(
                    Modifier
                        .selectableGroup()
                ) {
                    Row(
                        Modifier
                            .selectable(
                                selected = inspectionViewModel.profile == Profile.front,
                                onClick = {
                                    inspectionViewModel.profile = Profile.front
                                },
                                role = Role.RadioButton
                            )
                            .padding(4.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        RadioButton(
                            selected = inspectionViewModel.profile == Profile.front,
                            onClick = null
                        )
                        Text(
                            text = "Front",
                            modifier = Modifier.padding(start = 8.dp)
                        )
                    }
                    Row(
                        Modifier
                            .selectable(
                                selected = inspectionViewModel.profile == Profile.side,
                                onClick = {
                                    inspectionViewModel.profile = Profile.side
                                },
                                role = Role.RadioButton
                            )
                            .padding(4.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        RadioButton(
                            selected = inspectionViewModel.profile == Profile.side,
                            onClick = null
                        )
                        Text(
                            text = "Side",
                            modifier = Modifier.padding(start = 8.dp)
                        )
                    }
                }
            }
            SliderWithLabel("Height", heightInput, "cm", 100f..250f) { value ->
                heightInput = value
            }
            SliderWithLabel("Weight", weightInput, "kg", 50f..200f) { value ->
                weightInput = value
            }
            SliderWithLabel("Angle", thetaAngleInput, "°", -1.99f..1.99f) { value ->
                thetaAngleInput = value
            }
            Box(
                modifier = Modifier
                    .weight(1F, true)
                    .padding(4.dp)
                    .onSizeChanged {
                        imageSize = it
                    },
                contentAlignment = Alignment.Center
            ) {
                Card(
                    colors = CardDefaults.cardColors(
                        containerColor = Color.Transparent,
                        contentColor = Color.Transparent
                    ),
                    onClick = {
                        humanImagePickerLauncher.launch("image/*")
                    }
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
            Button(
                modifier = Modifier.padding(4.dp),
                onClick = { navController.navigate("poseInspection") }
            )
            { Text("Continue") }
        }
    }

    LaunchedEffect(sexInput, heightInput, weightInput, thetaAngleInput, inspectionViewModel.profile) {
        if (inspectionViewModel.refreshContour) {
            inspectionViewModel.generateIdealContour(context, sexInput, heightInput, weightInput, thetaAngleInput, inspectionViewModel.profile)
        } else {
            inspectionViewModel.refreshContour = true
        }
    }
}


@Composable
fun SliderWithLabel(name: String, value: Float, unit: String, valueRange: ClosedFloatingPointRange<Float>, onUpdateValue: (Float) -> Unit) {
    Row(
        Modifier
            .padding(4.dp)
            .fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            modifier = Modifier.weight(0.4F, true),
            text = "$name: ${value.format(2)}$unit"
        )
        Slider(
            modifier = Modifier.weight(0.6F, true),
            value = value,
            onValueChange = { onUpdateValue(it) },
            valueRange = valueRange
        )
    }
}

fun Float.format(digits: Int) = "%.${digits}f".format(this)