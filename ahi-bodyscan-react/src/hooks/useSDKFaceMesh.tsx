import { useEffect, useRef, RefObject } from 'react';
import { RPPGOnFrame } from 'vastmindz-web-sdk/dist/lib/RPPG.types';

export interface SDKFaceMeshConfig {
  canvasElement: RefObject<HTMLCanvasElement>;
  videoElement: RefObject<HTMLVideoElement>;
  frameData?: RPPGOnFrame;
  enabled: boolean;
}

// VastMindz SDK face landmarks are typically 68 points in x,y format (136 values)
const LANDMARK_CONNECTIONS = [
  // Jaw line (0-16)
  [0, 1], [1, 2], [2, 3], [3, 4], [4, 5], [5, 6], [6, 7], [7, 8],
  [8, 9], [9, 10], [10, 11], [11, 12], [12, 13], [13, 14], [14, 15], [15, 16],
  
  // Right eyebrow (17-21)
  [17, 18], [18, 19], [19, 20], [20, 21],
  
  // Left eyebrow (22-26)
  [22, 23], [23, 24], [24, 25], [25, 26],
  
  // Nose bridge (27-30)
  [27, 28], [28, 29], [29, 30],
  
  // Nose bottom (31-35)
  [31, 32], [32, 33], [33, 34], [34, 35], [35, 31],
  
  // Right eye (36-41)
  [36, 37], [37, 38], [38, 39], [39, 40], [40, 41], [41, 36],
  
  // Left eye (42-47)
  [42, 43], [43, 44], [44, 45], [45, 46], [46, 47], [47, 42],
  
  // Outer lips (48-59)
  [48, 49], [49, 50], [50, 51], [51, 52], [52, 53], [53, 54],
  [54, 55], [55, 56], [56, 57], [57, 58], [58, 59], [59, 48],
  
  // Inner lips (60-67)
  [60, 61], [61, 62], [62, 63], [63, 64], [64, 65], [65, 66], [66, 67], [67, 60],
];

function useSDKFaceMesh({ canvasElement, videoElement, frameData, enabled }: SDKFaceMeshConfig) {
  const animationFrameRef = useRef<number | undefined>(undefined);

  useEffect(() => {
    if (!enabled || !canvasElement.current || !videoElement.current || !frameData) {
      return;
    }

    const canvas = canvasElement.current;
    const video = videoElement.current;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const drawLandmarks = () => {
      // Get video's native and display dimensions
      const videoNativeWidth = video.videoWidth;
      const videoNativeHeight = video.videoHeight;
      const videoDisplayWidth = video.offsetWidth;
      const videoDisplayHeight = video.offsetHeight;

      if (!videoNativeWidth || !videoNativeHeight || !videoDisplayWidth || !videoDisplayHeight) {
        animationFrameRef.current = requestAnimationFrame(drawLandmarks);
        return;
      }

      // Set canvas to match display size
      if (canvas.width !== videoDisplayWidth || canvas.height !== videoDisplayHeight) {
        canvas.width = videoDisplayWidth;
        canvas.height = videoDisplayHeight;
        console.log(`Canvas resized to display size: ${canvas.width}x${canvas.height}`);
      }

      // Calculate scale for object-fit: cover behavior
      const videoAspectRatio = videoNativeWidth / videoNativeHeight;
      const displayAspectRatio = videoDisplayWidth / videoDisplayHeight;
      
      let scaleX: number = 1;
      let scaleY: number = 1;
      let offsetX: number = 0;
      let offsetY: number = 0;
      
      if (videoAspectRatio > displayAspectRatio) {
        // Video is wider than display - scale by height and crop width
        scaleY = videoDisplayHeight / videoNativeHeight;
        scaleX = scaleY;
        const scaledWidth = videoNativeWidth * scaleX;
        offsetX = (videoDisplayWidth - scaledWidth) / 2;
      } else {
        // Video is taller than display - scale by width and crop height
        scaleX = videoDisplayWidth / videoNativeWidth;
        scaleY = scaleX;
        const scaledHeight = videoNativeHeight * scaleY;
        offsetY = (videoDisplayHeight - scaledHeight) / 2;
      }

      // Clear canvas
      ctx.clearRect(0, 0, canvas.width, canvas.height);

      const landmarks = frameData.rppgTrackerData.landmarks;
      if (!landmarks || landmarks.length === 0) {
        animationFrameRef.current = requestAnimationFrame(drawLandmarks);
        return;
      }

      // Parse landmarks - SDK provides array of [x,y] arrays
      const points: Array<{ x: number; y: number }> = [];
      
      // Cast landmarks to correct runtime type (SDK types are incorrect)
      const landmarksArray = landmarks as unknown as Array<[number, number]>;
      
      for (let i = 0; i < landmarksArray.length; i++) {
        if (Array.isArray(landmarksArray[i]) && landmarksArray[i].length >= 2) {
          points.push({
            x: landmarksArray[i][0],
            y: landmarksArray[i][1],
          });
        }
      }

      
      // Debug: Log landmark count
      if (points.length > 0 && points.length !== 68) {
        console.log('Warning: Expected 68 landmarks, got', points.length);
      }

      // Transform points to display coordinates
      const transformedPoints = points.map(point => ({
        x: point.x * scaleX + offsetX,
        y: point.y * scaleY + offsetY,
      }));

      // Draw connections
      ctx.strokeStyle = '#00FF00';
      ctx.lineWidth = 2;
      ctx.globalAlpha = 0.7;

      LANDMARK_CONNECTIONS.forEach(([start, end]) => {
        if (start < transformedPoints.length && end < transformedPoints.length) {
          ctx.beginPath();
          ctx.moveTo(transformedPoints[start].x, transformedPoints[start].y);
          ctx.lineTo(transformedPoints[end].x, transformedPoints[end].y);
          ctx.stroke();
        }
      });

      // Draw points
      ctx.fillStyle = '#00FF00';
      ctx.globalAlpha = 1;
      transformedPoints.forEach((point, index) => {
        ctx.beginPath();
        ctx.arc(point.x, point.y, 3, 0, 2 * Math.PI);
        ctx.fill();
        
      });

      animationFrameRef.current = requestAnimationFrame(drawLandmarks);
    };

    drawLandmarks();

    return () => {
      if (animationFrameRef.current) {
        cancelAnimationFrame(animationFrameRef.current);
      }
    };
  }, [canvasElement, videoElement, frameData, enabled]);

  return null;
}

export default useSDKFaceMesh;