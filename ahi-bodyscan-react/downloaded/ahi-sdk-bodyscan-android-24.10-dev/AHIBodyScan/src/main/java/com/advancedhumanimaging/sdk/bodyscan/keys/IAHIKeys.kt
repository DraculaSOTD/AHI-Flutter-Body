package com.advancedhumanimaging.sdk.bodyscan.keys

import android.content.Context
import com.advancedhumanimaging.sdk.common.models.AHIKey

internal interface IAHIKeysUtil {
    fun keyAHIAppToken(context: Context): AHIKey
    fun keyAHILicenseToken(context: Context): AHIKey
}