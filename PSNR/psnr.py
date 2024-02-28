import cv2
import numpy as np

def psnr(img1, img2):
    psnr_values = []

    for i in range(img1.shape[2]):
        mse = np.mean((img1[:, :, i] - img2[:, :, i]) ** 2)
        if mse == 0:
            psnr_values.append(float('inf'))
        else:
            max_pixel = 255.0
            psnr_values.append(20 * np.log10(max_pixel / np.sqrt(mse)))

    return psnr_values

def main():
    # Wczytaj obrazy
    image_path1 = 'ref.bmp'  # Zastąp ścieżką do swojego obrazu referencyjnego
    image_path2 = 'v12_textureColorsCorrection2Mediana_1920x1080_yuv420p10le_0000.bmp'  # Zastąp ścieżką do swojego obrazu testowego

    img1 = cv2.imread(image_path1)
    img2 = cv2.imread(image_path2)

    # Sprawdź, czy obrazy mają takie same rozmiary
    if img1.shape != img2.shape:
        raise ValueError("Obrazy muszą mieć takie same rozmiary")

    # Oblicz PSNR dla każdej składowej koloru
    psnr_values = psnr(img1, img2)

    sum = 0

    for i, value in enumerate(psnr_values):
        sum += value
    
    print("PSNR: " + str(sum/3.0) + " dB")

if __name__ == "__main__":
    main()
