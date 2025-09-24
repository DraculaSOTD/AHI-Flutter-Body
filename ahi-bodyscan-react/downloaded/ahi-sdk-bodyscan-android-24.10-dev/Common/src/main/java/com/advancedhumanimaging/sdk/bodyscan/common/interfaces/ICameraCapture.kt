//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import com.advancedhumanimaging.sdk.bodyscan.common.Capture

/**
 * Interface declaration of Camera Capture service.
 */
interface ICameraCapture {

    /**
     * Set the camera capture configuration. The class implementation should declare the format of
     * the configuration in regard to its particulars.
     * @param config The camera capture configuration.
     * @return True if successfully set and the config is valid, otherwise false.
     */
    fun setConfig(
        config: Map<String, Any>,
    ): Boolean

    /**
     * Take a capture, based on the configuration setting.
     * @param meta Data to attach to the capture, which should include alignment angles and profile at the time of the capture request.
     * @return An array of captures. If there was an error, null will be returned.
     */
    suspend fun takeCapture(
        meta: Map<String, Any>,
    ): Array<Capture>?

}