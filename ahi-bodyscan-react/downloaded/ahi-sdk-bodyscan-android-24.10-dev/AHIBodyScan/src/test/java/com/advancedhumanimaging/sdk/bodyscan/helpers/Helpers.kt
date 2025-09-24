//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.helpers
import android.app.Application
import com.advancedhumanimaging.sdk.common.IAHIScan
import com.advancedhumanimaging.sdk.common.models.AHIResult
import com.advancedhumanimaging.sdk.multiscan.AHIMultiScan
import kotlin.coroutines.suspendCoroutine

private const val AHI_TOKEN = "testTOKEN123"
private const val AHI_INVAID_TIKEN = "invalid_token"

fun getValidSetupConfig(): Map<String, String> {
    return mapOf("TOKEN" to AHI_TOKEN)
}

fun getInvalidSetupConfig(): Map<String, String> {
    return mapOf("TOKEN" to AHI_INVAID_TIKEN)
}

suspend fun waitForMutliscanSetup(
    application: Application,
    config: Map<String, String>,
    scans: Array<IAHIScan>
): AHIResult<Unit> {
    return suspendCoroutine { continuation ->
        try {
            AHIMultiScan.setup(application, config, scans) {
                continuation.resumeWith(Result.success(it))
            }
        } catch (e: Exception) {
            continuation.resumeWith(Result.failure(e))
        }
    }
}