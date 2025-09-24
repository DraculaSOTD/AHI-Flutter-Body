package com.advancedhumanimaging.sdk.bodyscan.partcamera.example.view

import android.graphics.Bitmap
import android.graphics.ImageDecoder
import android.os.Build
import android.provider.MediaStore
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.layoutId
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.constraintlayout.compose.ConstraintLayout
import androidx.constraintlayout.compose.ConstraintSet
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.partcamera.example.viewmodel.AHICameraViewModel

@Composable
fun GalleryPreview(
    modifier: Modifier = Modifier.fillMaxSize(),
    navHostController: NavHostController,
    viewModel: AHICameraViewModel,
) {
    BoxWithConstraints(modifier = modifier) {
        ConstraintLayout(constraintSet = constraints(), modifier = modifier) {
            val context = LocalContext.current
            val bitmap = remember {
                mutableStateOf<Bitmap?>(null)
            }
            val uri = viewModel.uri
            uri?.let {
                if (Build.VERSION.SDK_INT < 28) {
                    bitmap.value = MediaStore.Images.Media.getBitmap(context.contentResolver, it)
                } else {
                    val source = ImageDecoder
                        .createSource(context.contentResolver, it)
                    bitmap.value = ImageDecoder.decodeBitmap(source)
                }
                bitmap.value?.let { bmp ->
                    Image(bitmap = bmp.asImageBitmap(), contentDescription = null, modifier = Modifier
                        .fillMaxSize()
                        .layoutId("image_view"))
                }
            }
            Button(
                modifier = Modifier.layoutId("back_button"),
                onClick = {
                    navHostController.popBackStack()
                }
            ) {
                Text(text = "Back")
            }
        }
    }
}

private fun constraints(): ConstraintSet {
    return ConstraintSet {
        val image_view = createRefFor("image_view")
        val back_button = createRefFor("back_button")
        constrain(image_view) {
            top.linkTo(parent.top, margin = 20.dp)
            start.linkTo(parent.start, margin = 20.dp)
            end.linkTo(parent.end, margin = 20.dp)
            bottom.linkTo(parent.bottom, margin = 20.dp)
        }
        constrain(back_button) {
            end.linkTo(parent.end, margin = 30.dp)
            bottom.linkTo(parent.bottom, margin = 30.dp)
        }
    }
}