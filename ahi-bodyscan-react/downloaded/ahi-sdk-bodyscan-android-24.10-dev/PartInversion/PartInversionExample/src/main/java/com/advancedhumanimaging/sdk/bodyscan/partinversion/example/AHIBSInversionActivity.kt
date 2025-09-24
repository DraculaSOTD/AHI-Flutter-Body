//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partinversion.example

import android.os.Bundle
import androidx.activity.compose.setContent
import androidx.fragment.app.FragmentActivity
import androidx.lifecycle.ViewModelProvider
import androidx.navigation.compose.rememberNavController
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.navigate.AHINavHost
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.viewmodel.AHIBSInversionViewModel

class AHIBSInversionActivity : FragmentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            val viewModel = ViewModelProvider(this)[AHIBSInversionViewModel::class.java]
            val navHostController = rememberNavController()
            AHINavHost(navHostController = navHostController, viewModel, supportFragmentManager)
        }
    }
}