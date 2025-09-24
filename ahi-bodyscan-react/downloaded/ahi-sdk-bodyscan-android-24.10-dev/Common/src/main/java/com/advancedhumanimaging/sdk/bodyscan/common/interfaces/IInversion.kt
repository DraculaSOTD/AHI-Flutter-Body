//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import android.content.Context
import android.net.Uri
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.common.models.AHIResult

/**
 * Interface declaration of Inversion service. This service generates a 3D mesh of a person based on their measurements and fitness level.
 */
interface IInversion {
    /**
     * Generate a 3D mesh as an obj file, of the person from their measurements.
     * @param name The name of mesh used to create URL.
     * @param sex The sex of the user.
     * @param heightCM The height of the user, in CMs.
     * @param weightKG The weight of the user, in KGs.
     * @param chestCM Chest measurement of the user, in CMs.
     * @param waistCM Waist measurement of the user, in CMs.
     * @param hipCM hip measurement of the user, in CMs
     * @param inseamCM inseam measurement of the user, in CMs.
     * @param fitness The fitness level of the user.
     * @param context Context of the calling application.
     * @param resources The service used for accessing all the needed resources.
     * @return An AHIResult containing the URI of the mesh
     */
    suspend fun invert(
        name: String,
        sex: SexType,
        heightCM: Double,
        weightKG: Double,
        chestCM: Double,
        waistCM: Double,
        hipCM: Double,
        inseamCM: Double,
        fitness: Double,
        context: Context,
        resources: IResources
    ): AHIResult<Uri>
}