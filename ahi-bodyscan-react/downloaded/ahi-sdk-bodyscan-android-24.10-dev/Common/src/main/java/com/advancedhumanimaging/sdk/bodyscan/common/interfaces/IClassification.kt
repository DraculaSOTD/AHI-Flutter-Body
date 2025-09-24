//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import android.content.Context
import com.advancedhumanimaging.sdk.bodyscan.common.CaptureGrouping
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.common.models.AHIResult

/**
 * Interface declaration of Classification service. This service takes silhouette image and other
 * meta-data to generate a scan result of the measurements.
 */
interface IClassification {

    /**
     * Generate scan measurement results from processed capture data.
     * @param context The application context.
     * @param resources The service used for accessing all the needed resources.
     * @param sex The sex of the user.
     * @param heightCM The height of the user, in CMs.
     * @param weightKG The weight of the user, in KGs.
     * @param captures Array of grouped front & side captures, with "silhouette", "joints", and "alignmentZ" values in meta.
     * @return Measurement results output. Schema should follow per https://docs.advancedhumanimaging.io/MultiScan%20SDK/BodyScan/Schemas/
     */
    suspend fun classify(
        context: Context,
        resources: IResources,
        sex: SexType,
        heightCM: Double,
        weightKG: Double,
        captures: Array<CaptureGrouping>,
        userAverage:Boolean = true
    ): AHIResult<Map<String, Any>?>

}