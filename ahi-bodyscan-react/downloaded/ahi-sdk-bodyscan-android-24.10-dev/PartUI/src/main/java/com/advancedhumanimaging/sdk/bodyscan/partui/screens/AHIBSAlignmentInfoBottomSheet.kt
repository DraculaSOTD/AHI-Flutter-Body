package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.TextView
import androidx.core.content.res.ResourcesCompat
import com.advancedhumanimaging.sdk.bodyscan.partui.R
import com.advancedhumanimaging.sdk.common.AHITheme
import com.google.android.material.bottomsheet.BottomSheetDialogFragment

class AHIBSAlignmentInfoBottomSheet(private val ahiTheme: AHITheme) : BottomSheetDialogFragment() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setStyle(STYLE_NORMAL, R.style.AHIBSBottomSheetDialogTheme)
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        return inflater.inflate(R.layout.fragment_ahibs_alignment_info_bottom_sheet, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        val closeButton = view.findViewById<Button>(R.id.close_button)
        val headingTextV = view.findViewById<TextView>(R.id.heading_text_view)
        val info1TextV = view.findViewById<TextView>(R.id.info_text_view1)
        val info2TextV = view.findViewById<TextView>(R.id.info_text_view2)
        val typeface = ahiTheme.baseFontResId?.let {
            try {
                ResourcesCompat.getFont(requireContext(), it)
            } catch (e: Exception) {
                null
            }
        }
        if (typeface != null) {
            closeButton.typeface = typeface
            headingTextV.typeface = typeface
            info1TextV.typeface = typeface
            info2TextV.typeface = typeface
        }
        closeButton.setOnClickListener { dismiss() }
    }

    companion object {
        const val TAG = "AHIBSAlignmentInfoBottomSheet"
    }

}