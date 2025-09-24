//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example.view

import androidx.activity.result.ActivityResultRegistry
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.foundation.selection.toggleable
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.layoutId
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.TextFieldValue
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.example.MainScreenState
import com.advancedhumanimaging.sdk.bodyscan.example.MainViewModel
import com.advancedhumanimaging.sdk.bodyscan.example.SdkOperationStatus
import com.advancedhumanimaging.sdk.bodyscan.example.components.AppTitle
import java.util.*

enum class SmoothingOption {
    NONE,
    NORMAL,
    OUTLIERS
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ScanCard(
    uiState: MainScreenState,
    mainViewModel: MainViewModel,
    navHostController: NavHostController,
    activityResultRegistry: ActivityResultRegistry,
) {
    if (uiState.sdkSetupState != SdkOperationStatus.SUCCESS ||
        uiState.sdkAuthZState != SdkOperationStatus.SUCCESS ||
        uiState.sdkResourceDownloadState != SdkOperationStatus.SUCCESS
    ) {
        return
    }
    val heightInput = remember { mutableStateOf(TextFieldValue("166")) }
    val weightInput = remember { mutableStateOf(TextFieldValue("66")) }
    val genderOptions = SexType.values().toList()
    val genderSelectIndex = remember {
        mutableStateOf(0)
    }
    var expanded by remember { mutableStateOf(false) }
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(8.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            AppTitle(label = "BODYSCAN")
            Spacer(modifier = Modifier.height(16.dp))
            Row(
                Modifier
                    .layoutId("sex")
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
                    Row {
                        genderOptions.forEachIndexed { index, sexType ->
                            Row(
                                Modifier
                                    .selectable(
                                        selected = sexType == genderOptions[genderSelectIndex.value],
                                        onClick = { genderSelectIndex.value = index },
                                        role = Role.RadioButton
                                    )
                                    .padding(horizontal = 8.dp),
                                verticalAlignment = Alignment.CenterVertically
                            ) {
                                RadioButton(
                                    selected = sexType == genderOptions[genderSelectIndex.value],
                                    onClick = null
                                )
                                Text(
                                    text = sexType.name.replaceFirstChar { it.uppercaseChar() },
                                    modifier = Modifier
                                        .padding(start = 8.dp)
                                )
                            }
                        }
                    }
                }
            }
            Spacer(modifier = Modifier.height(8.dp))
            TextField(
                value = heightInput.value,
                onValueChange = {
                    heightInput.value = it
                },
                label = {
                    Text(text = "Height input:")
                },
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
            )
            val hIsEmpty = heightInput.isValueEmpty("height")
            val hIsValid = heightInput.isValueValid("height")
            if (hIsEmpty.first) {
                Text(text = hIsEmpty.second, modifier = Modifier.background(Color.Red))
            } else if (hIsValid.first) {
                Text(text = hIsValid.second, modifier = Modifier.background(Color.Red))
            }
            Spacer(modifier = Modifier.height(8.dp))
            TextField(
                value = weightInput.value,
                onValueChange = {
                    weightInput.value = it
                },
                label = {
                    Text(text = "Weight input:")
                },
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
            )
            val wIsEmpty = weightInput.isValueEmpty("weight")
            val wIsValid = weightInput.isValueValid("weight")
            if (wIsEmpty.first) {
                Text(text = wIsEmpty.second, modifier = Modifier.background(Color.Red))
            } else if (wIsValid.first) {
                Text(text = wIsValid.second, modifier = Modifier.background(Color.Red))
            }
            Spacer(modifier = Modifier.height(8.dp))
            if (mainViewModel.lastClassificationResults.isNotEmpty()) {
                ExposedDropdownMenuBox(
                    expanded = expanded,
                    onExpandedChange = {
                        expanded = !expanded
                    }
                ) {
                    OutlinedTextField(
                        modifier = Modifier
                            .fillMaxWidth()
                            .menuAnchor(),
                        readOnly = true,
                        value = mainViewModel.smoothingOption.name.lowercase(Locale.ROOT).replaceFirstChar { it.uppercaseChar() },
                        onValueChange = { },
                        label = { Text("Smoothing") },
                        trailingIcon = {
                            ExposedDropdownMenuDefaults.TrailingIcon(
                                expanded = expanded
                            )
                        },
                        colors = ExposedDropdownMenuDefaults.textFieldColors()
                    )
                    ExposedDropdownMenu(
                        expanded = expanded,
                        onDismissRequest = {
                            expanded = false
                        }
                    ) {
                        SmoothingOption.values().forEach { option ->
                            DropdownMenuItem(
                                onClick = {
                                    mainViewModel.smoothingOption = option
                                    expanded = false
                                },
                                text = { Text(option.name.lowercase(Locale.ROOT).replaceFirstChar { it.uppercaseChar() }) }
                            )
                        }
                    }
                }
                Spacer(modifier = Modifier.height(8.dp))
            }
            Spacer(modifier = Modifier.height(8.dp))
            Row(
                Modifier
                    .fillMaxWidth()
                    .toggleable(
                        role = Role.Switch,
                        value = mainViewModel.isDebug,
                        onValueChange = { mainViewModel.isDebug = it },
                    )
            ) {
                Text("Debug Mode?")
                Spacer(Modifier.width(20.dp))
                Switch(
                    checked = mainViewModel.isDebug,
                    onCheckedChange = null
                )
            }
            Spacer(modifier = Modifier.height(8.dp))
            if (uiState.scanState == SdkOperationStatus.IN_PROGRESS) {
                CircularProgressIndicator()
            } else {
                Button(onClick = {
                    if (
                        !hIsEmpty.first &&
                        !hIsValid.first &&
                        !wIsEmpty.first &&
                        !wIsValid.first
                    ) {
                        val scanOptions = mapOf(
                            "cm_ent_height" to heightInput.value.text.toDouble(),
                            "kg_ent_weight" to weightInput.value.text.toDouble(),
                            "enum_ent_sex" to genderOptions[genderSelectIndex.value].name,
                            "debug_isDebug" to mainViewModel.isDebug,
                        )
                        mainViewModel.initiateScan(navHostController, activityResultRegistry, scanOptions)
                    }
                }) {
                    Text(text = "Initial Scan")
                }
            }
        }
    }
}

fun MutableState<TextFieldValue>.isValueEmpty(measurement: String): Pair<Boolean, String> {
    val isEmpty = this.value.text.isEmpty()
    return if (isEmpty) Pair(isEmpty, "${measurement.uppercase()} can not be empty.") else Pair(false, "")
}

fun MutableState<TextFieldValue>.isValueValid(measurement: String): Pair<Boolean, String> {
    val isValid = this.value.text.toDoubleOrNull() == null
    return if (isValid) Pair(true, "${measurement.uppercase()} can only be a number.") else Pair(false, "")
}