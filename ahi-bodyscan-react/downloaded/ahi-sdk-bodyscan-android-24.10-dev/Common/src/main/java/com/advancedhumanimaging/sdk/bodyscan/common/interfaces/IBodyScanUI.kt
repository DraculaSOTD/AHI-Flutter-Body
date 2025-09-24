//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import com.advancedhumanimaging.sdk.common.models.AHIResult

/**
 * Interface declaration of the BodyScan UI/UX. This implements, shows, and dismisses the BodyScan process
 * UI and UX flow.
 */
interface IBodyScanUI {

    /**
     * Present and start the UI of the BodyScan process.
     * @param options The options as provided to the `initiateScan()` method.
     * @return Raw measurement results or error.
     */
    suspend fun presentBodyScan(
        options: Map<String, Any>
    ): AHIResult<Map<String, Any>>

}