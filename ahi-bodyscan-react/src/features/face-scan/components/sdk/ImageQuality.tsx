import { useEffect, useState } from 'react';
import styled from 'styled-components';
import { ImageQualityFlags } from 'vastmindz-web-sdk/dist/lib/RPPGTracker.types';

const Container = styled.div`
  position: absolute;
  top: 100px;
  right: 20px;
  display: flex;
  gap: 8px;
  z-index: 10;
`;

const Star = styled.svg<{ active: boolean }>`
  width: 24px;
  height: 24px;
  color: ${props => props.active ? '#FFD700' : '#333'};
  transition: color 0.3s ease;
`;

type Props = {
  imageQualityFlags: ImageQualityFlags;
};

export const ImageQuality: React.FC<Props> = ({ imageQualityFlags }) => {
  const [imageQuality, setImageQuality] = useState<boolean[]>([]);

  useEffect(() => {
    const {
      brightColorFlag,
      illumChangeFlag,
      noiseFlag,
      sharpFlag,
    } = imageQualityFlags;

    const value = Object.values({
      brightColorFlag,
      illumChangeFlag,
      noiseFlag,
      sharpFlag,
    }).sort((a, b) => +b - +a);

    setImageQuality(value);
  }, [imageQualityFlags]);

  return (
    <Container>
      {imageQuality.map((value, key) => (
        <Star
          key={key}
          active={value}
          xmlns="http://www.w3.org/2000/svg"
          viewBox="0 0 24 24"
          fill="currentColor"
        >
          <path d="M12 2l3.09 6.26L22 9.27l-5 4.87 1.18 6.88L12 17.77l-6.18 3.25L7 14.14 2 9.27l6.91-1.01L12 2z"/>
        </Star>
      ))}
    </Container>
  );
};