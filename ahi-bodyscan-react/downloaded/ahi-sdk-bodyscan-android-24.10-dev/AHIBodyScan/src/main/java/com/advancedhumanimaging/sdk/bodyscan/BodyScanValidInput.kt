//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan

import android.os.Parcelable
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import kotlinx.parcelize.Parcelize

@Parcelize
data class BodyScanValidInput(
    val heightCm: Double,
    val weightKg: Double,
    val sex: SexType,
    val uid: String,
    val decryptionKey: String,
    val isDebug: Boolean,
    // To get enabled features(boolean) from cloud.
    var featuresEnabled: Map<String, Boolean> = mapOf(),
    // to get threshold from cloud config, set default as 0.94.
    var accuracyThreshold: Float = 0.94f,
) : Parcelable