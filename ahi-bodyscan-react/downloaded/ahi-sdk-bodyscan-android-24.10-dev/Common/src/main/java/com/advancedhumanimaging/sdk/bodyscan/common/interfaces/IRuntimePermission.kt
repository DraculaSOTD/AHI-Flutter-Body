//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

interface IRuntimePermission {
    /**
     * Check all permission are granted
     * */
    fun areAllPermissionsGranted(list: List<String>):Boolean

    /**
     * Get permission and display permission required in system dialog
     * */
    fun askPermissions(list: List<String>)
}