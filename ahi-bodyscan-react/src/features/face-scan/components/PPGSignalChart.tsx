import React, { useEffect, useRef } from 'react';
import styled from 'styled-components';

const ChartContainer = styled.div<{ $visible: boolean }>`
  position: absolute;
  top: 20px;
  right: 20px;
  width: 120px;
  height: 40px;
  z-index: 100;
  display: ${props => props.$visible ? 'block' : 'none'};
  transition: opacity 0.5s ease-in-out;
`;

const ChartCanvas = styled.canvas`
  width: 120px;
  height: 40px;
  position: absolute;
  top: 0;
  right: 0;
`;

interface PPGSignalChartProps {
  signal: number[];
  visible?: boolean;
  color?: string;
  lineWidth?: number;
  maxSamples?: number;
}

export const PPGSignalChart: React.FC<PPGSignalChartProps> = ({
  signal,
  visible = true,
  color = 'white',
  lineWidth = 1,
  maxSamples = 75,
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || !signal || signal.length === 0) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Set canvas actual dimensions
    canvas.width = 120;
    canvas.height = 40;

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.fillStyle = 'transparent';

    // Limit the number of entries in the signal
    const displaySignal = signal.slice(Math.max(signal.length - maxSamples, 0));

    if (displaySignal.length < 2) return;

    // Find min and max for normalization
    const maxSignal = Math.max(...displaySignal);
    const minSignal = Math.min(...displaySignal);
    const range = maxSignal - minSignal || 1;

    // Scale the x-axis to fit within the canvas width
    const xScale = canvas.width / displaySignal.length;

    // Draw the signal
    ctx.beginPath();
    ctx.strokeStyle = color;
    ctx.lineWidth = lineWidth;

    displaySignal.forEach((y, x) => {
      // Normalize the signal to the range of the canvas height
      const normalizedY = ((y - minSignal) / range) * canvas.height;
      
      if (x === 0) {
        ctx.moveTo(x * xScale, normalizedY);
      } else {
        ctx.lineTo(x * xScale, normalizedY);
      }
    });

    ctx.stroke();
  }, [signal, color, lineWidth, maxSamples]);

  return (
    <ChartContainer $visible={visible}>
      <ChartCanvas ref={canvasRef} />
    </ChartContainer>
  );
};