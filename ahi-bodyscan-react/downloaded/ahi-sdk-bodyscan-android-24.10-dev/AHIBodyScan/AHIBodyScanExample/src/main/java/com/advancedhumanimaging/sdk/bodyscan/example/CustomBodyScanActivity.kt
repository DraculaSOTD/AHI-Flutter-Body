//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example

import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color
import com.advancedhumanimaging.sdk.bodyscan.BodyScanCaptureActivity
import com.advancedhumanimaging.sdk.bodyscan.BodyScanCaptureViewModel

class CustomBodyScanActivity : BodyScanCaptureActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MainLayout(getBodyScanViewModel())
        }
    }

    @Composable
    private fun MainLayout(viewModel: BodyScanCaptureViewModel) {
        Surface(color = Color.White) {
        }
    }

}