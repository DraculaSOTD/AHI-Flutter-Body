//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcontour.example.ui.screens

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
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.unit.dp
import androidx.navigation.NavController
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partcontour.example.viewmodel.ContourViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun Home(navController: NavController, viewModel: ContourViewModel) {
    val context = LocalContext.current
    var sexInput by rememberSaveable { mutableStateOf(SexType.male) }
    var heightInput by rememberSaveable { mutableStateOf(190.0F) }
    var weightInput by rememberSaveable { mutableStateOf(70.0F) }
    var thetaAngleInput by rememberSaveable { mutableStateOf(0.0F) }
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
                                selected = viewModel.profile == Profile.front,
                                onClick = {
                                    viewModel.profile = Profile.front
                                },
                                role = Role.RadioButton
                            )
                            .padding(4.dp),
                        verticalAlignment = Alignment.CenterVertically
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
                                },
                                role = Role.RadioButton
                            )
                            .padding(4.dp),
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
                    .padding(4.dp),
                contentAlignment = Alignment.Center
            ) {
                Card(
                    colors = CardDefaults.cardColors(
                        containerColor = Color.Transparent,
                        contentColor = Color.Transparent
                    ),
                    onClick = {
                        navController.navigate("idealContourMask")
                    }
                ) {
                    val bitmap = if (viewModel.profile == Profile.front) viewModel.contourMaskFront else viewModel.contourMaskSide
                    if (bitmap != null) {
                        Image(
                            modifier = Modifier
                                .background(Color.Black),
                            bitmap = bitmap.asImageBitmap(),
                            contentDescription = "",
                            alignment = Alignment.Center,
                            contentScale = ContentScale.Fit,
                        )
                    }
                }
            }
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceEvenly
            ) {
                Button(
                    modifier = Modifier
                        .weight(1F, true)
                        .padding(8.dp),
                    onClick = { navController.navigate("scaledContour") }
                )
                { Text("Scaled Contour") }
                Button(
                    modifier = Modifier
                        .weight(1F, true)
                        .padding(8.dp),
                    onClick = { navController.navigate("optimalZones") }
                )
                { Text("Optimal Zones") }
            }
        }
    }

    LaunchedEffect(sexInput, heightInput, weightInput, thetaAngleInput, viewModel.profile) {
        viewModel.generateIdealContour(context, sexInput, heightInput, weightInput, thetaAngleInput)
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