import React, { useEffect, useRef } from 'react';
import styled from 'styled-components';

const CanvasContainer = styled.div`
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  z-index: 0;
  pointer-events: none;
`;

const ParticleCanvas = styled.canvas`
  width: 100%;
  height: 100%;
`;

interface ParticleBackgroundProps {
  particleColor?: string;
  backgroundColor?: string;
}

export const ParticleBackground: React.FC<ParticleBackgroundProps> = ({
  particleColor = '106, 0, 255', // Purple RGB
  backgroundColor = '#000000',
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const animationRef = useRef<number>(0);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const context = canvas.getContext('2d');
    if (!context) return;

    // Set canvas size
    const resizeCanvas = () => {
      canvas.width = window.innerWidth;
      canvas.height = window.innerHeight;
    };
    resizeCanvas();
    window.addEventListener('resize', resizeCanvas);

    // Particle system variables
    let displayWidth = canvas.width;
    let displayHeight = canvas.height;
    let wait = 1;
    let count = wait - 1;
    let numToAddEachFrame = 4;
    let particleList: any = {};
    let recycleBin: any = {};
    let particleAlpha = 1;
    let fLen = 256;
    let projCenterX = displayWidth / 2;
    let projCenterY = displayHeight / 2;
    let zMax = fLen - 2;
    let particleRad = 2.5;
    let sphereRad = 256;
    let sphereCenterX = 0;
    let sphereCenterY = 0;
    let sphereCenterZ = -3 - sphereRad;
    let zeroAlphaDepth = -750;
    let turnSpeed = (2 * Math.PI) / 1600;
    let turnAngle = 0;
    let randAccelX = 0.1;
    let randAccelY = 0.1;
    let randAccelZ = 0.1;
    let gravity = 0;
    const rgbString = `rgba(${particleColor}, `;

    // Particle functions
    const addParticle = (x0: number, y0: number, z0: number, vx0: number, vy0: number, vz0: number) => {
      let newParticle: any;

      if (recycleBin.first != null) {
        newParticle = recycleBin.first;
        if (newParticle.next != null) {
          recycleBin.first = newParticle.next;
          newParticle.next.prev = null;
        } else {
          recycleBin.first = null;
        }
      } else {
        newParticle = {};
      }

      if (particleList.first == null) {
        particleList.first = newParticle;
        newParticle.prev = null;
        newParticle.next = null;
      } else {
        newParticle.next = particleList.first;
        particleList.first.prev = newParticle;
        particleList.first = newParticle;
        newParticle.prev = null;
      }

      newParticle.x = x0;
      newParticle.y = y0;
      newParticle.z = z0;
      newParticle.velX = vx0;
      newParticle.velY = vy0;
      newParticle.velZ = vz0;
      newParticle.age = 0;
      newParticle.dead = false;
      newParticle.right = Math.random() < 0.5;

      return newParticle;
    };

    const recycle = (p: any) => {
      if (particleList.first === p) {
        if (p.next != null) {
          p.next.prev = null;
          particleList.first = p.next;
        } else {
          particleList.first = null;
        }
      } else {
        if (p.next == null) {
          p.prev.next = null;
        } else {
          p.prev.next = p.next;
          p.next.prev = p.prev;
        }
      }

      if (recycleBin.first == null) {
        recycleBin.first = p;
        p.prev = null;
        p.next = null;
      } else {
        p.next = recycleBin.first;
        recycleBin.first.prev = p;
        recycleBin.first = p;
        p.prev = null;
      }
    };

    // Animation loop
    const animate = () => {
      // Update canvas size if needed
      displayWidth = canvas.width;
      displayHeight = canvas.height;
      projCenterX = displayWidth / 2;
      projCenterY = displayHeight / 2;

      // Add new particles
      count++;
      if (count >= wait) {
        count = 0;
        for (let i = 0; i < numToAddEachFrame; i++) {
          const theta = Math.random() * 2 * Math.PI;
          const phi = Math.acos(Math.random() * 2 - 1);
          const x0 = sphereRad * Math.sin(phi) * Math.cos(theta);
          const y0 = sphereRad * Math.sin(phi) * Math.sin(theta);
          const z0 = sphereRad * Math.cos(phi);

          const p = addParticle(
            x0,
            sphereCenterY + y0,
            sphereCenterZ + z0,
            0.002 * x0,
            0.002 * y0,
            0.002 * z0
          );

          p.attack = 50;
          p.hold = 50;
          p.decay = 160;
          p.initValue = 0;
          p.holdValue = particleAlpha;
          p.lastValue = 0;
          p.stuckTime = 80 + Math.random() * 20;
          p.accelX = 0;
          p.accelY = gravity;
          p.accelZ = 0;
        }
      }

      // Update viewing angle
      turnAngle = (turnAngle + turnSpeed) % (2 * Math.PI);
      const sinAngle = Math.sin(turnAngle);
      const cosAngle = Math.cos(turnAngle);

      // Clear canvas
      context.fillStyle = backgroundColor;
      context.fillRect(0, 0, displayWidth, displayHeight);

      // Update and draw particles
      let p = particleList.first;
      while (p != null) {
        const nextParticle = p.next;

        p.age++;

        if (p.age > p.stuckTime) {
          p.velX += p.accelX + randAccelX * (Math.random() * 2 - 1);
          p.velY += p.accelY + randAccelY * (Math.random() * 2 - 1);
          p.velZ += p.accelZ + randAccelZ * (Math.random() * 2 - 1);

          p.x += p.velX;
          p.y += p.velY;
          p.z += p.velZ;
        }

        const rotX = cosAngle * p.x + sinAngle * (p.z - sphereCenterZ);
        const rotZ = -sinAngle * p.x + cosAngle * (p.z - sphereCenterZ) + sphereCenterZ;
        const m = fLen / (fLen - rotZ);
        p.projX = rotX * m + projCenterX;
        p.projY = p.y * m + projCenterY;

        // Update alpha
        if (p.age < p.attack + p.hold + p.decay) {
          if (p.age < p.attack) {
            p.alpha = ((p.holdValue - p.initValue) / p.attack) * p.age + p.initValue;
          } else if (p.age < p.attack + p.hold) {
            p.alpha = p.holdValue;
          } else if (p.age < p.attack + p.hold + p.decay) {
            p.alpha =
              ((p.lastValue - p.holdValue) / p.decay) *
                (p.age - p.attack - p.hold) +
              p.holdValue;
          }
        } else {
          p.dead = true;
        }

        // Check if particle is out of bounds
        const outsideTest =
          p.projX > displayWidth ||
          p.projX < 0 ||
          p.projY < 0 ||
          p.projY > displayHeight ||
          rotZ > zMax;

        if (outsideTest || p.dead) {
          recycle(p);
        } else {
          // Depth-dependent darkening
          let depthAlphaFactor = 1 - rotZ / zeroAlphaDepth;
          depthAlphaFactor =
            depthAlphaFactor > 1
              ? 1
              : depthAlphaFactor < 0
              ? 0
              : depthAlphaFactor;

          context.fillStyle = rgbString + depthAlphaFactor * p.alpha + ')';
          context.beginPath();
          context.arc(p.projX, p.projY, m * particleRad, 0, 2 * Math.PI, false);
          context.closePath();
          context.fill();
        }

        p = nextParticle;
      }

      animationRef.current = requestAnimationFrame(animate);
    };

    // Start animation
    animate();

    // Cleanup
    return () => {
      window.removeEventListener('resize', resizeCanvas);
      if (animationRef.current) {
        cancelAnimationFrame(animationRef.current);
      }
    };
  }, [particleColor, backgroundColor]);

  return (
    <CanvasContainer>
      <ParticleCanvas ref={canvasRef} />
    </CanvasContainer>
  );
};