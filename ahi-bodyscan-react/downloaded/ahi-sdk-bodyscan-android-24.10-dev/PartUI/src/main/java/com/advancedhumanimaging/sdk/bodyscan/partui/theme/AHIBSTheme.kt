//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partui.theme

import android.app.Activity
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.SideEffect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.Font
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.font.toFontFamily
import androidx.compose.ui.unit.sp
import androidx.core.view.ViewCompat
import com.advancedhumanimaging.sdk.common.AHIThemeStyle
import com.advancedhumanimaging.sdk.common.AHIThemes

@Composable
fun AHIBSTheme(
    content: @Composable () -> Unit
) {
    val darkColorScheme = darkColorScheme(
        primary = Color(AHIThemes.dark.primaryTextColor),
        secondary = Color(AHIThemes.dark.secondaryTextColor),
        background = Color(AHIThemes.dark.backgroundColor)
    )
    val lightColorScheme = lightColorScheme(
        primary = Color(AHIThemes.light.primaryTextColor),
        secondary = Color(AHIThemes.light.secondaryTextColor),
        background = Color(AHIThemes.light.backgroundColor)
    )
    val colorScheme = when (AHIThemes.style) {
        AHIThemeStyle.DARK -> darkColorScheme
        AHIThemeStyle.LIGHT -> lightColorScheme
        AHIThemeStyle.SYSTEM -> {
            if (isSystemInDarkTheme()) {
                darkColorScheme
            } else {
                lightColorScheme
            }
        }
    }
    val theme = AHIThemes.getCurrentTheme(isSystemInDarkTheme())
    val fontResId = theme.baseFontResId
    val fontSize = theme.baseFontSize.sp
    val typography = if (fontResId != null) {
        Typography(
            bodyLarge = TextStyle(
                fontFamily = Font(fontResId).toFontFamily(),
                fontWeight = FontWeight.Normal,
                fontSize = fontSize,
                lineHeight = 24.sp,
                letterSpacing = 0.5.sp
            ),
            labelLarge = Typography().labelLarge.copy(fontFamily = Font(fontResId).toFontFamily()),
            titleLarge = Typography().titleLarge.copy(fontFamily = Font(fontResId).toFontFamily()),
            headlineLarge = Typography().headlineLarge.copy(fontFamily = Font(fontResId).toFontFamily()),
            displayLarge = Typography().displayLarge.copy(fontFamily = Font(fontResId).toFontFamily()),
        )
    } else {
        Typography(
            bodyLarge = TextStyle(
                fontFamily = FontFamily.Default,
                fontWeight = FontWeight.Normal,
                fontSize = fontSize,
                lineHeight = 24.sp,
                letterSpacing = 0.5.sp
            ),
        )
    }
    val view = LocalView.current
    if (!view.isInEditMode) {
        SideEffect {
            (view.context as Activity).window.statusBarColor = android.graphics.Color.BLACK
            (view.context as Activity).window.navigationBarColor = android.graphics.Color.BLACK
            ViewCompat.getWindowInsetsController(view)?.isAppearanceLightStatusBars = false
        }
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = typography,
        content = content
    )
}