//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan

import androidx.activity.viewModels
import com.advancedhumanimaging.sdk.common.uidelegation.AHIScanCaptureActivity
import com.advancedhumanimaging.sdk.common.uidelegation.AHIScanViewModel

open class BodyScanCaptureActivity: AHIScanCaptureActivity<BodyScanValidInput>()  {
    override fun createViewModel(inputs: BodyScanValidInput): AHIScanViewModel<BodyScanValidInput> {
        val bodyScanViewModel: BodyScanCaptureViewModel by viewModels {
            BodyScanViewModelFactory(inputs)
        }
        return bodyScanViewModel
    }

    protected fun getBodyScanViewModel(): BodyScanCaptureViewModel {
        return scanViewModel as BodyScanCaptureViewModel
    }
}
