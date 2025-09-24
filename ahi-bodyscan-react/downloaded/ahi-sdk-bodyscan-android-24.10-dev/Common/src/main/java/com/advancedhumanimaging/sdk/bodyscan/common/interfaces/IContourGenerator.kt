//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution
import com.advancedhumanimaging.sdk.bodyscan.common.SexType

/**
 * Interface declaration of Contour generation service.
 */
interface IContourGenerator {

    /**
     * Generates an array of vertices/Points (on a 2D plane) of the ideal user contour, based on parameters.
     * The "ideal" contour is designated as the "best" position for the user to be in for the given user parameters.
     * The resulting array of Points can be used to draw a contour guide and be easily scaled.
     * @param context The application context.
     * @param resources The service used for accessing all the needed resources.
     * @param sex The sex of the user.
     * @param heightCM The height of the user, in CMs.
     * @param weightKG The weight of the user, in KGs.
     * @param imageSize The corresponding image resolution, which allows for aligning the contour to capture and image overlays.
     * @param alignmentZRadians The gravity detected to the device Z axis, See [reference](https://developer.apple.com/documentation/arkit/arconfiguration/worldalignment/gravity). Pure upright to gravity, Z = 0.
     * @param profile The orientation of the contour to generate, either front or side profile.
     * @return An array of 2D points outlining the generated user contour, or nil if failure.
     */
    suspend fun generateIdealContour(
        context: Context,
        resources: IResources,
        sex: SexType,
        heightCM: Float,
        weightKG: Float,
        imageSize: Resolution,
        alignmentZRadians: Float,
        profile: Profile,
    ): Array<PointF>?

    /**
     * Generate mask image of the contour for use in inspection and other compute purposes. This
     * is effectively a black and white image, where the contour and interior is white, and the background is black.
     * @param contour The generated contour, either as ideal or scaled variants.
     * @param imageSize The resolution of the image to render to contour mask to.
     * @return The rendered contour mask image, or nil if failure.
     */
    suspend fun generateContourMask(
        contour: Array<PointF>,
        imageSize: Resolution
    ): Bitmap?

    /**
     * Generates a new contour by scaling the source contour relative to detected person's pose joint locations.
     * @param contourIdeal The source contour, which should always be the generated "ideal" contour, to scale.
     * @param poseJoints The detected pose joints of the person.
     * @return A new contour that's base on the source contour and scaled with respect to the detected user pose joints.
     */
    suspend fun generateScaledContour(
        contourIdeal: Array<PointF>,
        poseJoints: Map<String, PointF>
    ): Array<PointF>?

    /**
     * Generates optimal zones for head and ankle in capture frame positions, expressed as 'zoneHead' and
     * 'zoneAnkles' rectangle area zones.
     * @param contourIdeal The source contour, which should always be the generated "ideal" contour.
     * @param imageSize The resolution of the image to render to contour mask to.
     * @return The top and bottom rectangle zone areas for optimal Body Scan capture, based on the "ideal" contour.
     */
    suspend fun generateOptimalZones(
        contourIdeal: Array<PointF>,
        imageSize: Resolution,
        accuracyThreshold:Float = 0.94f
    ): Map<String, RectF>?

    /**
     * Checks if the detected user pose joints are within the specific optimal zone.
     * @param zone If 'zoneAnkles', checks that the detected user pose ankle joints reside within the zone. If 'zoneHead', checks if the detected user pose ankle joints reside within the zone.
     * @param area The rectangle coordinates defining the zone, with respect to the image resolution used to generate the zones.
     * @param poseJoints The detected pose joints of the person.
     * @return True if the detected user pose joints in question reside within the optimal zone, otherwise false.
     */
    suspend fun isUserInOptimalZone(
        zone: String,
        area: RectF,
        poseJoints: Map<String, PointF>
    ): Boolean

}
