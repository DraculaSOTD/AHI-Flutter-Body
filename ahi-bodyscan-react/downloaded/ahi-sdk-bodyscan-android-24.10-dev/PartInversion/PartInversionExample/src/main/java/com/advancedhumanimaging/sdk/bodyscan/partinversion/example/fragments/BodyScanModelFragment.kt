//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partinversion.example.fragments

import android.graphics.PixelFormat
import android.net.Uri
import android.opengl.GLSurfaceView
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.advancedhumanimaging.sdk.bodyscan.partinversion.example.databinding.ItemBodyScanModelBinding
import com.threed.jpct.*
import com.threed.jpct.util.MemoryHelper
import java.io.File
import java.io.FileInputStream
import java.io.InputStream
import javax.microedition.khronos.opengles.GL10
import kotlin.math.PI

class BodyScanModelFragment(val uri: Uri, val height : Float) : Fragment() {

    private lateinit var binding: ItemBodyScanModelBinding
    private lateinit var renderer: MyRenderer
    private var fb: FrameBuffer? = null
    private lateinit var world: World
    private lateinit var sun: Light
    private lateinit var model: Object3D
    private val back = RGBColor(0, 0, 0, 0)
    private var rotation = 0f
    private var fps = 0

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        binding = ItemBodyScanModelBinding.inflate(inflater, container, false)
        binding.surfaceView.setZOrderOnTop(true)
        binding.surfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0)
        binding.surfaceView.holder.setFormat(PixelFormat.RGBA_8888)
        binding.rotateSlider.addOnChangeListener { _, value, _ ->
            //convert degrees from slider to radians for the model rotation method
            rotation = (value * (PI.toFloat() / 180F))
            //change the direction of rotation
            rotation *= -1
        }
        renderer = MyRenderer(uri)
        binding.surfaceView.setRenderer(renderer)
        return binding.root
    }

    override fun onPause() {
        super.onPause()
        binding.surfaceView.onPause()
    }

    override fun onResume() {
        super.onResume()
        binding.surfaceView.onResume()
    }

    inner class MyRenderer(private val uri: Uri) : GLSurfaceView.Renderer {
        private var time = System.currentTimeMillis()
        override fun onSurfaceCreated(p0: GL10?, p1: javax.microedition.khronos.egl.EGLConfig?) {
            world = World()
            world.setAmbientLight(100, 100, 100)
            sun = Light(world)
            sun.setIntensity(148F, 148F, 148F)

            val obj: InputStream = FileInputStream(File(uri.getPath()))
            val heightAsFraction = 1F - (height / 255F)
            val scaleFactor = (heightAsFraction / 2) + 0.7F
            val objArray = Loader.loadOBJ(obj, null, scaleFactor)

            model = objArray[0]
            model.rotateX(Math.PI.toFloat())
            model.rotateMesh()
            model.clearRotation()
            model.strip()
            model.build()
            world.addObject(model)
            val cam: Camera = world.camera
            cam.moveCamera(Camera.CAMERA_MOVEOUT, 2.75F)
            cam.moveCamera(Camera.CAMERA_MOVEUP, 1.3F)
            cam.rotateCameraX(0.15F)
            cam.fov = cam.minFOV
            val sv = SimpleVector()
            sv.set(model.transformedCenter)
            sv.y -= 100f
            sv.x -= 100f
            sv.z -= 300f
            sun.position = sv
            MemoryHelper.compact()
        }

        override fun onSurfaceChanged(gl: GL10, w: Int, h: Int) {
            fb?.dispose()
            fb = FrameBuffer(gl, w, h)
        }

        override fun onDrawFrame(gl: GL10) {
            if (rotation != 0F) {
                //clear previous rotation so we keep model rotation in sync with slider
                model.clearRotation()
                model.rotateY(rotation)
            }
            fb?.clear(back)
            world.renderScene(fb)
            world.draw(fb)
            fb?.display()
            if (System.currentTimeMillis() - time >= 1000) {
                Logger.log(fps.toString() + "fps", Logger.LL_VERBOSE)
                fps = 0
                time = System.currentTimeMillis()
            }
            fps++
        }
    }
}