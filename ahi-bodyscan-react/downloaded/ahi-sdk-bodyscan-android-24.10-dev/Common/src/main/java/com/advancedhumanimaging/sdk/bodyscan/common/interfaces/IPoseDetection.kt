//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import android.graphics.Bitmap
import android.graphics.PointF
import com.advancedhumanimaging.sdk.bodyscan.common.DetectionType

/**
 * Interface declaration of Pose Detection service. This service with attempt to detect a person joints (ankle, hands, head, etc...) from a photo image.
 */
interface IPoseDetection {

    /**
     * Detect pose joints from a given capture image.
     * @param type The type of detection to perform.
     * @param capture Reference to the image capture being inspected.
     * @return Detected pose joints in the format:
     * ```json
     * {
     *    // List of points per a face if 'face' or 'faceAndBody' type:
     *   "face":[
     *              {
     *                  "CentroidHeadTop": Point,
     *                  "CentroidNeck": Point,
     *                  "FacePosition": Point,
     *                  "FaceSize": Point
     *              }
     *          ],
     *   // List of points per a body if 'body' or 'faceAndBody' type:
     *   "body":[
     *              {
     *                  "CentroidRightAnkle": Point,
     *                  "CentroidLeftAnkle": Point,
     *                  "CentroidRightKnee": Point,
     *                  "CentroidLeftKnee": Point,
     *                  "CentroidRightHip": Point,
     *                  "CentroidLeftHip": Point,
     *                  "CentroidRightHand": Point,
     *                  "CentroidLeftHand": Point,
     *                  "CentroidRightElbow": Point,
     *                  "CentroidLeftElbow": Point,
     *                  "CentroidRightShoulder": Point,
     *                  "CentroidLeftShoulder": Point,
     *                  "CentroidNose": Point,
     *              }
     *          ]
     * }
     * ```
     */
    suspend fun detect(
        type: DetectionType,
        bitmap: Bitmap,
    ): Map<String, List<Map<String, PointF>>>?
}