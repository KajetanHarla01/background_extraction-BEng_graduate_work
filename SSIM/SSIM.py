import cv2
from skimage.metrics import structural_similarity as ssim

def calculate_ssim(img1, img2):
    # Oblicz SSIM dla każdej składowej koloru (R, G, B)
    ssim_values = []
    for i in range(img1.shape[2]):
        channel_ssim, _ = ssim(img1[:, :, i], img2[:, :, i], full=True)
        ssim_values.append(channel_ssim)

    # Uśrednij wyniki dla każdej składowej koloru
    avg_ssim = sum(ssim_values) / len(ssim_values)
    return avg_ssim

def main():
    # Wczytaj obrazy
    image_path1 = 'ref.bmp'  # Zastąp ścieżką do swojego obrazu referencyjnego
    image_path2 = 'v12_textureSredniaGlebia_1920x1080_yuv420p10le_0000.bmp'  # Zastąp ścieżką do swojego obrazu testowego

    img1 = cv2.imread(image_path1)
    img2 = cv2.imread(image_path2)

    # Sprawdź, czy obrazy mają takie same rozmiary
    if img1.shape != img2.shape:
        raise ValueError("Obrazy muszą mieć takie same rozmiary")

    # Oblicz SSIM uwzględniając składowe kolorów
    ssim_value = calculate_ssim(img1, img2)

    print(f"SSIM między obrazem referencyjnym a testowym: {ssim_value}")

if __name__ == "__main__":
    main()
