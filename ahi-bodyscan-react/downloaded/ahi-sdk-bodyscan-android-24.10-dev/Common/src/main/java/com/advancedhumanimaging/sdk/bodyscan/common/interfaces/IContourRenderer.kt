//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import android.graphics.Color
import android.graphics.Point
import android.media.Image
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution

/**
 * Interface declaration of Contour rendering service.
 */
interface IContourRenderer {

    /**
     * Renders the provided contour to image for various display needs.
     * @param points The generated contour.
     * @param imageSize The resolution of the image to render to contour to.
     * @param backgroundColor The background color, everything not contour or it's interior, which would usually be semi or fully transparent when contour is overlay of camera.
     * @param foregroundColor The foreground color, which is the contour interior, which would usually be fully transparent when contour is overlay of camera.
     * @param lineSolid If true, the contour outline draw should be solid line (no dashes).
     * @param lineColor The colour of the contour outline.
     * @param lineDashColor The alternating color of the contour outline, if `lineSolid` parameter is false.
     * @return Image of the rendered contour, of nil if there was a failure.
     */
    fun drawDisplayContour(
        points: Array<Point>,
        imageSize: Resolution,
        backgroundColor: Color,
        foregroundColor: Color,
        lineSolid: Boolean,
        lineColor: Color,
        lineDashColor: Color
    ): Image?

}
