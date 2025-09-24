//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partinversion.example.view

import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.fragment.app.FragmentManager
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.fragments.BodyScanModelFragment
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.fragments.FragmentContainer
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.ui.theme.BodyscanTheme
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.viewmodel.AHIBSInversionViewModel

@Composable
@OptIn(ExperimentalMaterial3Api::class)
fun InversionResultView(
    naviHostController: NavHostController,
    viewModel: AHIBSInversionViewModel,
    fragmentManager: FragmentManager
) {
    BodyscanTheme {
        Scaffold(
            topBar = {
                CenterAlignedTopAppBar(
                    title = {
                        Text("Result")
                    },
                    navigationIcon = {
                        IconButton(onClick = { naviHostController.navigateUp() }) {
                            Icon(imageVector = Icons.Filled.ArrowBack, contentDescription = "Back")
                        }
                    }
                )
            }, content = { padding ->
                FragmentContainer(
                    modifier = Modifier.fillMaxSize().padding(padding),
                    fragmentManager = fragmentManager,
                    commit = {
                        add(
                            it,
                            BodyScanModelFragment(viewModel.meshLocation!!, viewModel.height)
                        )
                    }
                )
            })
    }
}