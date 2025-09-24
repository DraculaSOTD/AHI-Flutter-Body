//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.helpers

import android.content.Context
import android.media.AudioManager
import android.media.SoundPool
import com.advancedhumanimaging.sdk.bodyscan.R

enum class AHISounds {
    SOUND_ALIGNMENT,
    SOUND_COUNTDOWN,
    SOUND_CAPTURE
}

class AHISoundUtils(
    applicationContext: Context
) {
    private var soundPool: SoundPool = SoundPool(AHISounds.values().size, AudioManager.STREAM_ALARM, 0)
    private var soundIds: List<Int> = listOf(
        soundPool.load(applicationContext, R.raw.alignment_sound, 0),
        soundPool.load(applicationContext, R.raw.countdown_sound, 0),
        soundPool.load(applicationContext, R.raw.capture_sound,0)
    )

    fun play(sound: AHISounds) {
        soundPool.play(soundIds.get(sound.ordinal), 1.0f, 1.0f, 1, 0, 1.0f)
    }
}