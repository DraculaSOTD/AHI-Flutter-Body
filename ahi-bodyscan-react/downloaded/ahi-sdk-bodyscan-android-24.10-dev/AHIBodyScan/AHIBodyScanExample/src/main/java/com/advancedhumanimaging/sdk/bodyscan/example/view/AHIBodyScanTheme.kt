//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example.view

import androidx.compose.foundation.*
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.rememberLazyGridState
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.RadioButton
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.saveable.Saver
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.layout.layoutId
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.constraintlayout.compose.ConstraintLayout
import androidx.constraintlayout.compose.ConstraintSet
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.example.MainViewModel
import com.advancedhumanimaging.sdk.bodyscan.example.R
import com.advancedhumanimaging.sdk.bodyscan.example.navigate.NavRoutes
import com.advancedhumanimaging.sdk.common.AHITheme
import com.advancedhumanimaging.sdk.common.AHIThemeStyle
import com.advancedhumanimaging.sdk.common.AHIThemes
import com.chillibits.composenumberpicker.HorizontalNumberPicker
import java.util.*


@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AHIBodyScanTheme(
    navHostController: NavHostController,
    mainViewModel: MainViewModel,
    modifier: Modifier = Modifier
) {
    Scaffold(
        modifier = modifier
    ) { contentPadding ->
        BoxWithConstraints(modifier = Modifier.padding(contentPadding)) {
            ConstraintLayout(modifier = Modifier.fillMaxSize(), constraintSet = constraints()) {
                val themePropertiesModifier = Modifier.layoutId("view")
                when (mainViewModel.styleApplied) {
                    AHIThemeStyle.LIGHT.ordinal ->
                        ThemeProperties(modifier = themePropertiesModifier, viewModel = mainViewModel, currentTheme = AHIThemes.light)
                    AHIThemeStyle.DARK.ordinal ->
                        ThemeProperties(modifier = themePropertiesModifier, viewModel = mainViewModel, currentTheme = AHIThemes.dark)
                    else -> if (isSystemInDarkTheme())
                        ThemeProperties(modifier = themePropertiesModifier, viewModel = mainViewModel, currentTheme = AHIThemes.dark)
                    else
                        ThemeProperties(modifier = themePropertiesModifier, viewModel = mainViewModel, currentTheme = AHIThemes.light)
                }
                ExtendedFloatingActionButton(
                    modifier = Modifier.layoutId("back"),
                    text = { Text(text = "HOME") },
                    icon = { Icon(Icons.Filled.ArrowBack, contentDescription = null) },
                    onClick = {
                        navHostController.navigate(NavRoutes.HOME_SCREEN)
                    }
                )
            }
        }
    }
}

private fun constraints(): ConstraintSet {
    return ConstraintSet {
        val view = createRefFor("view")
        val settings = createRefFor("back")
        constrain(view) {
            top.linkTo(parent.top)
            start.linkTo(parent.start)
            end.linkTo(parent.end)
        }
        constrain(settings) {
            bottom.linkTo(parent.bottom, margin = 8.dp)
            end.linkTo(parent.end, margin = 8.dp)
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ThemeProperties(
    modifier: Modifier,
    viewModel: MainViewModel,
    currentTheme: AHITheme
) {
    Column(
        modifier = modifier
            .verticalScroll(rememberScrollState())
            .fillMaxSize()
            .padding(30.dp)
    ) {
        val colors = listOf(
            Color.Blue,
            Color.Black,
            Color.Gray,
            Color.Cyan,
            Color.White,
            Color.Red,
            Color.Yellow,
            Color.Green,
            Color.Magenta
        )
        val styles = AHIThemeStyle.values().map { style -> style.name.lowercase().replaceFirstChar { it.titlecase(Locale.ROOT) } }
        SingleChooseButtonGroup(
            title = "Choose Style",
            radioOptions = styles,
            viewModel = viewModel
        )
        Spacer(modifier = Modifier.height(16.dp))
        ColourButton(
            themeElement = "Backgound",
            colors = colors,
            currentColor = Color(currentTheme.backgroundColor),
            onColorSelected = {
                currentTheme.backgroundColor = it.toArgb()
            }
        )
        ColourButton(
            themeElement = "Primary Text",
            colors = colors,
            currentColor = Color(currentTheme.primaryTextColor),
            onColorSelected = {
                currentTheme.primaryTextColor = it.toArgb()
            }
        )
        ColourButton(
            themeElement = "Secondary Text",
            colors = colors,
            currentColor = Color(currentTheme.secondaryTextColor),
            onColorSelected = {
                currentTheme.secondaryTextColor = it.toArgb()
            }
        )
        ColourButton(
            themeElement = "Primary Tint",
            colors = colors,
            currentColor = Color(currentTheme.primaryTintColor),
            onColorSelected = {
                currentTheme.primaryTintColor = it.toArgb()
            }
        )
        ColourButton(
            themeElement = "Secondary Tint",
            colors = colors,
            currentColor = Color(currentTheme.secondaryTintColor),
            onColorSelected = {
                currentTheme.secondaryTintColor = it.toArgb()
            }
        )
        Spacer(modifier = Modifier.height(8.dp))
        Row(modifier = Modifier.padding(4.dp), horizontalArrangement = Arrangement.Center, verticalAlignment = Alignment.CenterVertically) {
            Text(modifier = Modifier.padding(4.dp), text = "Font Size")
            HorizontalNumberPicker(
                height = 32.sp.value.dp,
                min = 8,
                max = 40,
                default = currentTheme.baseFontSize.toInt(),
                onValueChange = {
                    currentTheme.baseFontSize = it.toFloat()
                }
            )
        }
        val fontList =
            listOf(Pair("Default", null), Pair("Salsa", R.font.salsa), Pair("Balthazar", R.font.balthazar), Pair("Aclonica", R.font.aclonica))
        var expanded by remember {
            mutableStateOf(false)
        }
        var selectedFont by remember {
            mutableStateOf(fontList.find { it.second == currentTheme.baseFontResId } ?: fontList[0])
        }
        Spacer(modifier = Modifier.height(8.dp))
        ExposedDropdownMenuBox(
            modifier = Modifier.padding(4.dp),
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
                value = selectedFont.first,
                onValueChange = { },
                label = { Text("Font") },
                trailingIcon = {
                    ExposedDropdownMenuDefaults.TrailingIcon(
                        expanded = expanded
                    )
                },
            )
            ExposedDropdownMenu(
                expanded = expanded,
                onDismissRequest = {
                    expanded = false
                }
            ) {
                fontList.forEach { option ->
                    DropdownMenuItem(
                        onClick = {
                            selectedFont = option
                            currentTheme.baseFontResId = option.second
                            expanded = false
                        },
                        text = { Text(option.first) }
                    )
                }
            }
        }
    }
}

@Composable
fun SingleChooseButtonGroup(
    title: String,
    radioOptions: List<String>,
    viewModel: MainViewModel,
) {
    Column(Modifier.selectableGroup()) {
        Text(
            text = title,
            style = MaterialTheme.typography.bodyLarge.merge(),
            modifier = Modifier.padding(4.dp)
        )
        radioOptions.forEachIndexed { index, text ->
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .selectable(
                        selected = (viewModel.styleApplied == index),
                        onClick = {
                            viewModel.styleApplied = index
                            AHIThemeStyle
                                .values()
                                .getOrNull(index)
                                ?.let { AHIThemes.style = it }
                        },
                        role = Role.RadioButton
                    )
                    .padding(4.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                RadioButton(selected = (viewModel.styleApplied == index), onClick = null)
                Text(
                    text = text,
                    style = MaterialTheme.typography.bodyLarge.merge(),
                    modifier = Modifier.padding(4.dp)
                )
            }
        }
    }
}

@Composable
fun ColourButton(themeElement: String, colors: List<Color>, onColorSelected: (Color) -> Unit, currentColor: Color) {
    var colorPickerOpen by rememberSaveable { mutableStateOf(false) }
    var currentlySelected by rememberSaveable(saver = colourSaver()) { mutableStateOf(currentColor) }
    Box(
        modifier = Modifier
            .padding(8.dp)
            .fillMaxWidth(0.8f)
            .clip(RoundedCornerShape(20))
            .border(
                2.dp,
                MaterialTheme.colorScheme.onBackground.copy(alpha = 0.75f),
                RoundedCornerShape(20)
            )
            .clickable {
                colorPickerOpen = true
            }
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(8.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = themeElement,
            )
            Canvas(
                modifier = Modifier
                    .size(30.dp)
                    .clip(RoundedCornerShape(20))
                    .border(
                        1.dp,
                        MaterialTheme.colorScheme.onBackground.copy(alpha = 0.75f),
                        RoundedCornerShape(20)
                    )
                    .background(currentlySelected)
                    .clickable {
                        colorPickerOpen = true
                    }
            ) {}
        }

    }
    if (colorPickerOpen) {
        ColorDialog(
            colorList = colors,
            onDismiss = { colorPickerOpen = false },
            currentlySelected = currentlySelected,
            onColorSelected = {
                currentlySelected = it
                onColorSelected(it)
            }
        )
    }
}


@Composable
private fun ColorDialog(
    colorList: List<Color>,
    onDismiss: (() -> Unit),
    currentlySelected: Color,
    onColorSelected: ((Color) -> Unit), // when the save button is clicked
) {
    val gridState = rememberLazyGridState()

    AlertDialog(
        shape = RoundedCornerShape(20.dp),
        containerColor = MaterialTheme.colorScheme.background,
        titleContentColor = MaterialTheme.colorScheme.outline,
        onDismissRequest = onDismiss,
        text = {
            LazyVerticalGrid(
                columns = GridCells.Fixed(3),
                state = gridState
            ) {
                items(colorList.size) {
                    var borderWidth = 0.dp
                    if (currentlySelected == colorList[it]) {
                        borderWidth = 2.dp
                    }

                    Canvas(modifier = Modifier
                        .padding(8.dp)
                        .clip(RoundedCornerShape(20.dp))
                        .border(
                            borderWidth,
                            MaterialTheme.colorScheme.onBackground.copy(alpha = 0.75f),
                            RoundedCornerShape(20.dp)
                        )
                        .background(colorList[it])
                        .requiredSize(70.dp)
                        .clickable {
                            onColorSelected(colorList[it])
                            onDismiss()
                        }
                    ) {
                    }
                }
            }
        },
        confirmButton = {}
    )
}

fun colourSaver() = Saver<MutableState<Color>, String>(
    save = { state -> state.value.toHexString() },
    restore = { value -> mutableStateOf(value.toColor()) }
)

fun Color.toHexString(): String {
    return String.format(
        "#%02x%02x%02x%02x", (this.alpha * 255).toInt(),
        (this.red * 255).toInt(), (this.green * 255).toInt(), (this.blue * 255).toInt()
    )
}

fun String.toColor(): Color {
    return Color(android.graphics.Color.parseColor(this))
}