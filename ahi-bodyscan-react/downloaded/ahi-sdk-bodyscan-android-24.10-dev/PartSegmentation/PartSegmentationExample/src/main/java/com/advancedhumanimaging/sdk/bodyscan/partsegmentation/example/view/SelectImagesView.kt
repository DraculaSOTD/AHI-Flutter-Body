//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.view

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.ImageDecoder
import android.os.Build
import android.provider.MediaStore
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.Button
import androidx.compose.material.Text
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.RadioButton
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.navigate.NavRoute
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.ui.theme.BodyscanTheme
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.viewmodel.AHIBSSegmentationViewModel

@Composable
@OptIn(ExperimentalMaterial3Api::class)
fun SelectImagesView(
    naviHostController: NavHostController,
    viewModel: AHIBSSegmentationViewModel,
) {
    val context = LocalContext.current
    val emptyBitmap = Bitmap.createBitmap(1, 1, Bitmap.Config.ARGB_8888)
    var humanBitmap by remember {
        mutableStateOf(
            if (viewModel.humanBitmap.sameAs(emptyBitmap)) {
                // if the bmp has not been initialised, load it here
                getHumanBitmap(viewModel.profile, context)
            } else {
                viewModel.humanBitmap
            }
        )
    }
    var contourBitmap by remember {
        mutableStateOf(
            if (viewModel.contourBitmap.sameAs(emptyBitmap)) {
                // if the bmp has not been initialised, load it here
                getContourBitmap(viewModel.profile, context)
            } else {
                viewModel.contourBitmap
            }
        )
    }

    val humanImagePickerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent(),
        onResult = { imageUri ->
            imageUri?.let {
                val bitmap = if (Build.VERSION.SDK_INT < 28) {
                    MediaStore.Images.Media.getBitmap(context.contentResolver, it)
                } else {
                    val source = ImageDecoder.createSource(context.contentResolver, it)
                    ImageDecoder.decodeBitmap(source) { decoder, _, _ ->
                        decoder.isMutableRequired = true
                    }
                }
                humanBitmap = Bitmap.createScaledBitmap(bitmap, 720, 1280, true)
                viewModel.humanBitmap = humanBitmap
                viewModel.contourBitmap = contourBitmap
                naviHostController.navigate(NavRoute.JOINTS_SCREEN)
            }
        }
    )
    val contourImagePickerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent(),
        onResult = { imageUri ->
            imageUri?.let {
                val bitmap = if (Build.VERSION.SDK_INT < 28) {
                    MediaStore.Images.Media.getBitmap(context.contentResolver, it)
                } else {
                    val source = ImageDecoder.createSource(context.contentResolver, it)
                    ImageDecoder.decodeBitmap(source) { decoder, _, _ ->
                        decoder.isMutableRequired = true
                    }
                }
                contourBitmap = Bitmap.createScaledBitmap(bitmap, 720, 1280, true)
            }
        }
    )
    BodyscanTheme {
        Column(
            modifier = Modifier.height(IntrinsicSize.Min).fillMaxWidth()
                .verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Row(Modifier.selectableGroup()) {
                Row(
                    Modifier
                        .selectable(
                            selected = viewModel.profile == Profile.front,
                            onClick = {
                                viewModel.profile = Profile.front
                                humanBitmap = getHumanBitmap(viewModel.profile, context)
                                contourBitmap = getContourBitmap(viewModel.profile, context)
                            },
                            role = Role.RadioButton
                        ).padding(8.dp),
                    verticalAlignment = Alignment.CenterVertically,
                ) {
                    RadioButton(
                        selected = viewModel.profile == Profile.front,
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
                            selected = viewModel.profile == Profile.side,
                            onClick = {
                                viewModel.profile = Profile.side
                                humanBitmap = getHumanBitmap(viewModel.profile, context)
                                contourBitmap = getContourBitmap(viewModel.profile, context)
                            },
                            role = Role.RadioButton
                        )
                        .padding(8.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    RadioButton(
                        selected = viewModel.profile == Profile.side,
                        onClick = null
                    )
                    Text(
                        text = "Side",
                        modifier = Modifier.padding(start = 8.dp)
                    )
                }
            }
            Box(contentAlignment = Alignment.Center) {
                Card(
                    modifier = Modifier.padding(8.dp),
                    colors = CardDefaults.cardColors(
                        containerColor = Color.Transparent,
                        contentColor = Color.Transparent
                    ),
                    onClick = {
                        contourImagePickerLauncher.launch("image/*")
                    }
                ) {
                    Image(
                        bitmap = contourBitmap.asImageBitmap(),
                        contentDescription = "",
                        alignment = Alignment.Center,
                        contentScale = ContentScale.Fit,
                    )
                }
            }
            Box(contentAlignment = Alignment.Center) {
                Card(
                    modifier = Modifier.padding(8.dp),
                    colors = CardDefaults.cardColors(
                        containerColor = Color.Transparent,
                        contentColor = Color.Transparent
                    ),
                    onClick = {
                        humanImagePickerLauncher.launch("image/*")
                    }
                ) {
                    Image(
                        bitmap = humanBitmap.asImageBitmap(),
                        contentDescription = "",
                        alignment = Alignment.Center,
                        contentScale = ContentScale.Fit,
                    )
                }
            }
            Box(contentAlignment = Alignment.Center) {
                Card(
                    modifier = Modifier.padding(8.dp),
                    colors = CardDefaults.cardColors(
                        containerColor = Color.Transparent,
                        contentColor = Color.Transparent
                    ),
                    onClick = {
                        humanImagePickerLauncher.launch("image/*")
                    }
                ) {
                    Button(
                        onClick = {
                            viewModel.humanBitmap = humanBitmap
                            viewModel.contourBitmap = contourBitmap
                            naviHostController.navigate(NavRoute.RESULT_SCREEN)
                        },
                        modifier = Modifier.padding(16.dp)
                    ) {
                        Text(text = "Segment")
                    }
                }
            }
        }
    }
}

private fun getHumanBitmap(profile: Profile, context: Context): Bitmap {
    val humanImg = context.assets.open(if (profile == Profile.front) "front.png" else "side.png")
    return Bitmap.createScaledBitmap(BitmapFactory.decodeStream(humanImg), 720, 1280, true)
}

private fun getContourBitmap(profile: Profile, context: Context): Bitmap {
    val contourImg =
        context.assets.open(if (profile == Profile.front) "contourMaskFront.png" else "contourMaskSideManual.png")
    return Bitmap.createScaledBitmap(BitmapFactory.decodeStream(contourImg), 720, 1280, true)
}