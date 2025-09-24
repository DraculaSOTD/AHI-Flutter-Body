//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.common.models.AHIResult

/**
 * Interface declaration of Segmentation service. This service extracts a silhouette of a person from a photo capture.
 */
interface ISegmentation {

    /**
     * Extract a silhouette, black-and-white image, of the person from a photo capture.
     * @param capture Reference to the image capture being inspected.
     * @param contourMask The contour mask image, which is expected to be the scaled contour silhouette.
     * @param profile The current profile, either front or side profile.
     * @param poseJoints The detected pose joints of the person in the capture.
     * @return Black-and-white image of a silhouette of the person.
     */
    suspend fun segment(
        capture: Bitmap,
        contourMask: Bitmap,
        profile: Profile,
        poseJoints: Map<String, PointF>,
        context: Context,
        resources: IResources
    ): AHIResult<Bitmap>
}
