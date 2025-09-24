package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.vectorResource
import androidx.compose.ui.unit.dp
import com.advancedhumanimaging.sdk.bodyscan.partui.R

@Composable
fun AHIBSLayout(
    infoAction: () -> Unit,
    closeAction: () -> Unit,
    content: @Composable () -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            IconButton(modifier = Modifier
                .padding(4.dp),
                onClick = { infoAction() }
            ) {
                Icon(
                    imageVector = ImageVector.vectorResource(id = R.drawable.ic_outline_help_outline_24),
                    tint = Color.White,
                    contentDescription = "Info"
                )
            }
            IconButton(modifier = Modifier
                .padding(4.dp),
                onClick = { closeAction() }
            ) {
                Icon(
                    imageVector = ImageVector.vectorResource(id = R.drawable.ic_baseline_cancel_24),
                    tint = Color.White,
                    contentDescription = "Close"
                )
            }
        }
        Box(
            modifier = Modifier
                .weight(1F, true),
        ) {
            Box(
                modifier = Modifier
                    .aspectRatio(9F / 16F, false)
                    .align(Alignment.Center)
            ) {
                content()
            }
        }
    }
}