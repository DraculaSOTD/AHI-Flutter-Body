#!/usr/bin/env python3
"""
Convert CoreML models to TensorFlow.js format for web usage.

Requirements:
pip install coremltools tensorflow tensorflowjs Pillow
"""

import os
import sys
import json
import coremltools as ct
import tensorflow as tf
import tensorflowjs as tfjs
from pathlib import Path

# Paths
MODELS_DIR = Path("../BodyScanML/models")
OUTPUT_DIR = Path("../public/models")

# Model mappings
MODELS_TO_CONVERT = {
    "joint_detection": {
        "source": MODELS_DIR / "joint/MYQJointModel.mlmodel",
        "output": OUTPUT_DIR / "joint_detection",
        "description": "14-point pose estimation model"
    },
    "segmentation": {
        "source": MODELS_DIR / "segmentation/MyqSegmentationModel.mlmodel",
        "output": OUTPUT_DIR / "segmentation",
        "description": "Person segmentation model"
    },
    "classification_v3_male": {
        "source": MODELS_DIR / "classification/MYQClassificationModelV3m.mlmodel",
        "output": OUTPUT_DIR / "classification/v3_male",
        "description": "Male body measurement classification"
    },
    "classification_v3_female": {
        "source": MODELS_DIR / "classification/MYQClassificationModelV3f.mlmodel",
        "output": OUTPUT_DIR / "classification/v3_female",
        "description": "Female body measurement classification"
    }
}

def convert_coreml_to_tf(coreml_path, tf_path):
    """Convert a CoreML model to TensorFlow format."""
    print(f"Loading CoreML model from {coreml_path}")
    
    # Load the CoreML model
    model = ct.models.MLModel(str(coreml_path))
    
    # Get model specs
    spec = model.get_spec()
    print(f"Model description: {spec.description}")
    
    # Convert to TensorFlow
    # Note: This is a simplified conversion. Real conversion might need more configuration
    # based on the specific model architecture
    try:
        # For newer coremltools versions
        tf_model = ct.converters.convert(
            model,
            convert_to="tensorflow",
            minimum_deployment_target=ct.target.iOS14
        )
    except:
        # Fallback for older versions or different conversion path
        print("Direct conversion failed. Attempting alternative method...")
        # This would require implementing a custom converter based on the model architecture
        raise NotImplementedError("Custom conversion needed for this model type")
    
    # Save TensorFlow model
    tf.saved_model.save(tf_model, str(tf_path))
    print(f"Saved TensorFlow model to {tf_path}")
    
    return tf_model

def convert_tf_to_tfjs(tf_path, tfjs_path):
    """Convert TensorFlow model to TensorFlow.js format."""
    print(f"Converting TensorFlow model to TensorFlow.js format")
    
    # Convert to TensorFlow.js
    tfjs.converters.convert_tf_saved_model(
        str(tf_path),
        str(tfjs_path),
        quantization_dtype_map={'uint8': '*', 'uint16': '*'}  # Enable quantization for smaller size
    )
    
    print(f"Saved TensorFlow.js model to {tfjs_path}")

def create_model_metadata(model_name, model_info, output_path):
    """Create metadata file for the converted model."""
    metadata = {
        "name": model_name,
        "description": model_info["description"],
        "version": "1.0.0",
        "format": "tfjs",
        "source": "CoreML",
        "converted": str(Path.cwd()),
        "files": {
            "model": "model.json",
            "weights": [f for f in os.listdir(output_path) if f.endswith('.bin')]
        }
    }
    
    with open(output_path / "metadata.json", "w") as f:
        json.dump(metadata, f, indent=2)

def main():
    """Main conversion pipeline."""
    # Create output directory
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    # Track conversion results
    results = []
    
    for model_name, model_info in MODELS_TO_CONVERT.items():
        print(f"\n{'='*60}")
        print(f"Converting {model_name}")
        print(f"{'='*60}")
        
        try:
            # Check if source model exists
            if not model_info["source"].exists():
                print(f"WARNING: Model not found at {model_info['source']}")
                results.append({
                    "model": model_name,
                    "status": "not_found",
                    "error": "Source model file not found"
                })
                continue
            
            # Create output directories
            temp_tf_path = OUTPUT_DIR / f"temp_tf_{model_name}"
            model_info["output"].mkdir(parents=True, exist_ok=True)
            
            # Convert CoreML to TensorFlow
            tf_model = convert_coreml_to_tf(model_info["source"], temp_tf_path)
            
            # Convert TensorFlow to TensorFlow.js
            convert_tf_to_tfjs(temp_tf_path, model_info["output"])
            
            # Create metadata
            create_model_metadata(model_name, model_info, model_info["output"])
            
            # Clean up temporary TensorFlow model
            import shutil
            shutil.rmtree(temp_tf_path)
            
            results.append({
                "model": model_name,
                "status": "success",
                "output": str(model_info["output"])
            })
            
        except Exception as e:
            print(f"ERROR converting {model_name}: {str(e)}")
            results.append({
                "model": model_name,
                "status": "failed",
                "error": str(e)
            })
    
    # Print summary
    print(f"\n{'='*60}")
    print("CONVERSION SUMMARY")
    print(f"{'='*60}")
    
    for result in results:
        status_emoji = "✅" if result["status"] == "success" else "❌"
        print(f"{status_emoji} {result['model']}: {result['status']}")
        if "error" in result:
            print(f"   Error: {result['error']}")
    
    # Save conversion report
    with open(OUTPUT_DIR / "conversion_report.json", "w") as f:
        json.dump(results, f, indent=2)

if __name__ == "__main__":
    # Note: This script is a template. Actual CoreML to TensorFlow conversion
    # might require model-specific handling based on the architecture.
    # For production use, consider using alternative approaches such as:
    # 1. Re-training the models directly in TensorFlow
    # 2. Using ONNX as an intermediate format
    # 3. Implementing the model architecture directly in TensorFlow.js
    
    print("Model Conversion Script")
    print("Note: This is a template script. Actual conversion may require")
    print("model-specific modifications based on the CoreML architecture.")
    print("\nFor body scan models, recommended approach:")
    print("1. Use pre-trained TensorFlow.js models for pose detection (PoseNet/MoveNet)")
    print("2. Re-implement measurement algorithms in JavaScript")
    print("3. Use BodyPix for segmentation instead of custom model")
    
    # Uncomment to run actual conversion
    # main()