//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partinversion

import com.advancedhumanimaging.sdk.bodyscan.common.SexType

internal object InversionJNI {
    // Used to load the 'inversion' library on application startup.
    init {
        System.loadLibrary("inversion")
    }

    external fun invert(
        sex: SexType,
        heightCM: Double,
        weightKG: Double,
        chestCM: Double,
        waistCM: Double,
        hipCM: Double,
        inseamCM: Double,
        fitness: Double,
        cvModelsMale: Map<String, Pair<ByteArray, Int>>,
        cvModelsFemale: Map<String, Pair<ByteArray, Int>>
    ): String
}