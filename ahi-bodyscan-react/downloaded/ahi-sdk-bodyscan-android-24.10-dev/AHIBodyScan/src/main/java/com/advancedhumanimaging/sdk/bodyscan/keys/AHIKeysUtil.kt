package com.advancedhumanimaging.sdk.bodyscan.keys

import android.content.Context
import com.advancedhumanimaging.sdk.common.helpers.key.AHIKeyToolHelper
import com.advancedhumanimaging.sdk.common.helpers.key.AHIKeyType
import com.advancedhumanimaging.sdk.common.models.AHIKey

internal class AHIKeysUtil(imageName: String, delimiters: String) : IAHIKeysUtil {
    private val multiScanImageName = imageName
    private val delimiters = delimiters
    override fun keyAHIAppToken(context: Context): AHIKey {
        return AHIKeyToolHelper.extractAHIKeys(
            context = context,
            imageName = multiScanImageName,
            ahiKeyEnum = AHIKeyType.values()[0],
            delimiters = delimiters
        )
    }

    override fun keyAHILicenseToken(context: Context): AHIKey {
        return AHIKeyToolHelper.extractAHIKeys(
            context = context,
            imageName = multiScanImageName,
            ahiKeyEnum = AHIKeyType.values()[1],
            delimiters = delimiters
        )
    }
}
