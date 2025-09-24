//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import com.advancedhumanimaging.sdk.bodyscan.common.InspectionResult
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.common.models.AHIResult

/**
 * Interface declaration of Pose Inspection service.
 */
interface IPoseInspection {

    /**
     * For a given capture, inspect that the person in in the relevant pose.
     * @param capture Reference to the image capture being inspected.
     * @param contourMask The contour mask image.
     * @param optimalZones The optimal zones, as calculated from the ideal contour.
     * @param profile The current profile, either front or side profile.
     * @param poseJoints The detected pose joints of the person in the capture in the format
     * ```json
     * {
     *   "face": list of points per a face,
     *   "body": list of points per a body,
     * }
     * ```
     * @return Inspection result in the format:
     * ```json
     * {
     *   "face": InspectionResult,
     *   // Only for Front profile inspection:
     *   "la: InspectionResult,              // LA = Left Arm
     *   "ra": InspectionResult,             // RA = Right Arm
     *   "ll": InspectionResult,             // LL = Left Leg Ankle
     *   "rl": InspectionResult,             // RL = Right Leg Ankle
     *   // Only for Side profile inspection:
     *   "ub": InspectionResult,             // UB = Upper Body (wrist by body side, not in front, behind, or above).
     *   "lb": InspectionResult,             // LB = Lower Body (feet together and both within the contour).
     * }
     * ```
     */
    suspend fun inspect(
        capture: Bitmap,
        contourMask: Bitmap,
        optimalZones: Map<String, RectF>,
        profile: Profile,
        poseJoints: Map<String, List<Map<String, PointF>>>
    ): AHIResult<Map<String, InspectionResult>>

    /**
     * Provides a reference specification for evaluating inspection result is acceptable or not.
     * @param profile The current profile, either front or side profile.
     * @param inspectionResult Inspection result details.
     * @return True if the given inspection result is acceptable to proceed with generating measurements, otherwise false.
     */
    suspend fun isInContour(
        profile: Profile,
        inspectionResult: Map<String, InspectionResult>
    ): Boolean

}
