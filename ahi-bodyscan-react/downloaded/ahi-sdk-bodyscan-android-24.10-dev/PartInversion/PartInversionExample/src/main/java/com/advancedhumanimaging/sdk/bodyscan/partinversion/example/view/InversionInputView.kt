//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partinversion.example.view

import android.content.Context
import android.widget.Toast
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewModelScope
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partinversion.Inversion
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.navigate.NavRoute
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.ui.theme.BodyscanTheme
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.viewmodel.AHIBSInversionViewModel
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

@Composable
@OptIn(ExperimentalMaterial3Api::class)
fun InversionInputView(
    naviHostController: NavHostController,
    viewModel: AHIBSInversionViewModel,
) {
    val context = LocalContext.current
    BodyscanTheme {
        Scaffold(
            topBar = {
                CenterAlignedTopAppBar(
                    title = {
                        Text("Input")
                    }
                )
            }, content = { padding ->
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .verticalScroll(rememberScrollState())
                        .padding(padding),
                    verticalArrangement = Arrangement.Center,
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    TextField(
                        value = viewModel.fileName,
                        onValueChange = {
                            viewModel.fileName = it
                        },
                        label = { Text("File name") }
                    )
                    SexRadioButtons(viewModel.sex) { value -> viewModel.sex = value }
                    SliderWithLabel("Height", viewModel.height, "cm") { value ->
                        viewModel.height = value
                    }
                    SliderWithLabel("Weight", viewModel.weight, "kg") { value ->
                        viewModel.weight = value
                    }
                    SliderWithLabel("Chest", viewModel.chest, "cm") { value ->
                        viewModel.chest = value
                    }
                    SliderWithLabel("Waist", viewModel.waist, "cm") { value ->
                        viewModel.waist = value
                    }
                    SliderWithLabel("Hip", viewModel.hip, "cm") { value -> viewModel.hip = value }
                    SliderWithLabel("Inseam", viewModel.inseam, "cm") { value ->
                        viewModel.inseam = value
                    }
                    Text(text = "Fitness: " + viewModel.fitness)
                    Slider(
                        value = viewModel.fitness,
                        onValueChange = { viewModel.fitness = it },
                        valueRange = -5f..10f
                    )
                    GenerateMeshButton(naviHostController, viewModel, context)
                }
            })
    }
}

@Composable
fun SliderWithLabel(name: String, value: Float, unit: String, onUpdateValue: (Float) -> Unit) {
    Text(text = "$name: $value$unit")
    Slider(
        value = value,
        onValueChange = { onUpdateValue(it) },
        valueRange = -50f..255f
    )
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SexRadioButtons(currentSex: SexType, onUpdateValue: (SexType) -> Unit) {
    Row(Modifier.selectableGroup()) {
        Row(
            Modifier
                .selectable(
                    selected = currentSex == SexType.male,
                    onClick = {
                        onUpdateValue(SexType.male)
                    },
                    role = Role.RadioButton
                )
                .padding(8.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            RadioButton(
                selected = currentSex == SexType.male,
                onClick = null
            )
            androidx.compose.material.Text(
                text = "Male",
                modifier = Modifier.padding(start = 8.dp)
            )
        }
        Row(
            Modifier
                .selectable(
                    selected = currentSex == SexType.female,
                    onClick = {
                        onUpdateValue(SexType.female)
                    },
                    role = Role.RadioButton
                )
                .padding(8.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            RadioButton(
                selected = currentSex == SexType.female,
                onClick = null
            )
            androidx.compose.material.Text(
                text = "Female",
                modifier = Modifier.padding(start = 8.dp)
            )
        }
    }
}

@Composable
fun GenerateMeshButton(
    naviHostController: NavHostController,
    viewModel: AHIBSInversionViewModel,
    context: Context
) {
    Button(
        onClick = {
            viewModel.viewModelScope.launch(Dispatchers.Main) {
                val result = Inversion().invert(
                    viewModel.fileName,
                    viewModel.sex,
                    viewModel.height.toDouble(),
                    viewModel.weight.toDouble(),
                    viewModel.chest.toDouble(),
                    viewModel.waist.toDouble(),
                    viewModel.hip.toDouble(),
                    viewModel.inseam.toDouble(),
                    viewModel.fitness.toDouble(),
                    context,
                    Resources()
                )
                if (result.isSuccess) {
                    viewModel.meshLocation = result.getOrNull()
                    naviHostController.navigate(NavRoute.RESULT_SCREEN)
                } else {
                    Toast.makeText(
                        context,
                        "Failed: " + result.error().toString(),
                        Toast.LENGTH_LONG
                    ).show()
                }
            }
        },
        modifier = Modifier.padding(16.dp),
    ) {
        Text("Generate 3D mesh")
    }
}

