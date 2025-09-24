//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partalignment.example.view

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.sp

@Composable
fun DisplayLabel(
    modifier: Modifier = Modifier,
    title: String,
    data: Double,
) {
    Column(modifier = modifier) {
        Row(modifier = modifier) {
            Text(
                text = "$title: ",
                style = TextStyle(Color.Red, fontWeight = FontWeight.Bold, fontSize = 30.sp)
            )
            Text(
                text = String.format("%.7f", data),
                style = TextStyle(Color.Blue, fontWeight = FontWeight.Bold, fontSize = 25.sp)
            )
        }
    }
}