//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import android.content.Context
import android.net.Uri
import com.advancedhumanimaging.sdk.common.models.AHIResult

enum class AHIBSResourceType {
    AHIBSResourceTypeSVR,
    AHIBSResourceTypeVI,
    AHIBSResourceTypeVD,
    AHIBSResourceTypeVVD,
    AHIBSResourceTypeVF,
    AHIBSResourceTypeVVF,
    AHIBSResourceTypeML
}

/**
 * Interface declaration of Resources service.
 */

interface IResources {

    /** Get resource for given name. This might require resources to be downloaded first.
     * @param name Resource name.
     * @param type The resource type to load.
     * @param context Context used to locate files directory
     *@return Buffer of resource located and loaded, otherwise error.
     */
    suspend fun getResource(
        name: String,
        type: AHIBSResourceType,
        context: Context
    ): AHIResult<ByteArray>

    /** Free the resource from memory.
     * @param name Resource name.
     */
    fun releaseResource(name: String)
}