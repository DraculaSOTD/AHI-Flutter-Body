//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CutCornerShape
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.res.vectorResource
import androidx.compose.ui.unit.dp
import com.advancedhumanimaging.sdk.bodyscan.partui.R
import com.advancedhumanimaging.sdk.bodyscan.partui.components.AHIBSPrompt
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.AHIBSRed
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.ANIMATION_DELAY
import kotlinx.coroutines.delay

enum class CriticalErrorState {
    PHONE_TOO_HIGH, PHONE_TOO_LOW, PHONE_TOO_HIGH_INDETERMINATE, PHONE_TOO_LOW_INDETERMINATE, PERSON_MISSING, ANKLES_MISSING
}

@Composable
fun AHIBSCriticalError(
    modifier: Modifier, state: CriticalErrorState, button1Action: () -> Unit, button2Action: () -> Unit
) {
    val infiniteTransition = rememberInfiniteTransition()
    val iconAlpha by infiniteTransition.animateFloat(
        initialValue = 0F, targetValue = 1F, animationSpec = infiniteRepeatable(
            animation = tween(ANIMATION_DELAY.toInt(), delayMillis = ANIMATION_DELAY.toInt(), easing = LinearEasing), repeatMode = RepeatMode.Reverse
        )
    )
    var animateIcon by remember { mutableStateOf(false) }
    Box(
        modifier = modifier.background(Color.Black.copy(alpha = 0.7F))
    ) {
        var message = listOf("")
        var buttonText1 = ""
        var buttonText2 = ""
        when (state) {
            CriticalErrorState.PHONE_TOO_LOW, CriticalErrorState.PHONE_TOO_LOW_INDETERMINATE -> {
                message = stringResource(id = R.string.phone_higher_message).split("\n")
                buttonText1 = stringResource(id = R.string.phone_height_retry_message)
                buttonText2 = stringResource(id = R.string.phone_height_tips)
            }

            CriticalErrorState.PHONE_TOO_HIGH, CriticalErrorState.PHONE_TOO_HIGH_INDETERMINATE -> {
                message = stringResource(id = R.string.phone_lower_message).split("\n")
                buttonText1 = stringResource(id = R.string.phone_height_retry_message)
                buttonText2 = stringResource(id = R.string.phone_height_tips)
            }

            CriticalErrorState.PERSON_MISSING -> {
                message = stringResource(id = R.string.head_ankle_invisible_message).split("\n")
                buttonText1 = stringResource(id = R.string.try_again)
                buttonText2 = stringResource(id = R.string.head_ankle_invisible_tips)
            }

            CriticalErrorState.ANKLES_MISSING -> {
                message = stringResource(id = R.string.ankle_invisible_message).split("\n")
                buttonText1 = stringResource(id = R.string.phone_placement_retry_message)
                buttonText2 = stringResource(id = R.string.phone_placement_tips)
            }
        }
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .align(Alignment.Center),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            AHIBSPrompt(
                modifier = Modifier,
                texts = message,
                backgroundColor = AHIBSRed,
                icon = ImageVector.vectorResource(id = R.drawable.ic_stop),
                iconShape = CutCornerShape(25.dp),
                animateIcon = animateIcon,
                iconAnimationAlpha = iconAlpha
            )
            Spacer(modifier = Modifier.size(24.dp))
            OutlinedButton(
                onClick = { button1Action() }, colors = ButtonDefaults.outlinedButtonColors(
                    containerColor = Color.White, contentColor = Color.Black
                )
            ) {
                Text(text = buttonText1)
            }
            Spacer(modifier = Modifier.size(4.dp))
            OutlinedButton(
                onClick = { button2Action() }, colors = ButtonDefaults.outlinedButtonColors(
                    containerColor = Color.Transparent, contentColor = Color.White
                )
            ) {
                Text(text = buttonText2)
            }
        }
    }

    LaunchedEffect(key1 = true) {
        delay(800)
        animateIcon = true
    }
}