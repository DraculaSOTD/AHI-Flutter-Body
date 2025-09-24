//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partui.components

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.Icon
import androidx.compose.material3.LocalTextStyle
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.RectangleShape
import androidx.compose.ui.graphics.Shape
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.LineHeightStyle
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp

@Composable
fun AHIBSPrompt(
    modifier: Modifier = Modifier,
    texts: List<String>,
    backgroundColor: Color,
    icon: ImageVector? = null,
    iconShape: Shape? = null,
    animateIcon: Boolean = false,
    iconAnimationAlpha: Float? = null
) {
    var viewSize by remember { mutableStateOf(IntSize.Zero) }
    Column(
        modifier = modifier
            .fillMaxWidth()
            .onSizeChanged {
                viewSize = it
            },
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        val textSize = viewSize.width / 7F
        val infiniteTransition = rememberInfiniteTransition()
        val alpha by infiniteTransition.animateFloat(
            initialValue = 0.1F,
            targetValue = 1F,
            animationSpec = infiniteRepeatable(
                animation = tween(100, delayMillis = 100, easing = LinearEasing),
                repeatMode = RepeatMode.Reverse
            )
        )
        if (icon != null) {
            Icon(
                modifier = Modifier
                    .clip(iconShape ?: RectangleShape)
                    .size(100.dp)
                    .background(backgroundColor)
                    .padding(20.dp)
                    .alpha(if (animateIcon) (iconAnimationAlpha ?: alpha) else 1F),
                imageVector = icon,
                tint = Color.White,
                contentDescription = ""
            )
            Spacer(modifier = Modifier.size(16.dp))
        }
        texts.forEachIndexed { index, text ->
            Text(
                modifier = Modifier
                    .background(backgroundColor)
                    .padding(start = 4.dp, end = 4.dp),
                text = text,
                color = Color.White,
                style = LocalTextStyle.current.copy(
                    fontWeight = FontWeight.ExtraBold,
                    fontSize = with(LocalDensity.current) { textSize.toSp() },
                    textAlign = TextAlign.Center,
                    lineHeightStyle = LineHeightStyle(alignment = LineHeightStyle.Alignment.Center, trim = LineHeightStyle.Trim.Both)
                ),
            )
            if (index != texts.lastIndex) Spacer(modifier = Modifier.size(4.dp))
        }
    }
}