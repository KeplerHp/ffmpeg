import cv2
import numpy as np
import ffmpeg

def extract_frames_from_h264(h264_file, output_dir):
    cap = cv2.VideoCapture(h264_file)
    frame_count = 0
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        output_path = f"{output_dir}/frame_{frame_count:04d}.png"
        cv2.imwrite(output_path, frame)
        frame_count += 1
    cap.release()
    return frame_count

def load_bgra_file(file_path, width, height):
    with open(file_path, 'rb') as f:
        bgra_data = np.frombuffer(f.read(), dtype=np.uint8)
    return bgra_data.reshape((height, width, 4))

def calculate_fsnr(original_frame, reconstructed_frame):
    signal_power = np.sum(np.square(original_frame.astype(np.float64)))
    noise_power = np.sum(np.square(original_frame.astype(np.float64) - reconstructed_frame.astype(np.float64)))
    
    if noise_power == 0:
        return float('inf')  # 如果没有噪声，FSNR 是无穷大
    
    fsnr = 10 * np.log10(signal_power / noise_power)
    return fsnr

def compute_fsnr(bgra_file, h264_file, width, height, output_dir):
    num_frames = extract_frames_from_h264(h264_file, output_dir)
    total_fsnr = 0.0
    
    for i in range(num_frames):
        original_frame = load_bgra_file(bgra_file, width, height)
        
        reconstructed_frame_path = f"{output_dir}/frame_{i:04d}.png"
        reconstructed_frame = cv2.imread(reconstructed_frame_path, cv2.IMREAD_UNCHANGED)

        # 检查图像是否加载为 BGRA 格式，如果没有，转换格式
        if reconstructed_frame.shape[2] == 3:
            reconstructed_frame = cv2.cvtColor(reconstructed_frame, cv2.COLOR_BGR2BGRA)

        # 确保分辨率一致
        if original_frame.shape != reconstructed_frame.shape:
            print(f"Original frame shape: {original_frame.shape}")
            print(f"Reconstructed frame shape: {reconstructed_frame.shape}")
            raise ValueError("Original and reconstructed frames have different shapes.")
        
        fsnr = calculate_fsnr(original_frame, reconstructed_frame)
        print(f"Frame {i}: FSNR = {fsnr:.2f} dB")
        total_fsnr += fsnr
    
    avg_fsnr = total_fsnr / num_frames
    return avg_fsnr


if __name__ == "__main__":
    bgra_file = "../input/input_1080.bgra"
    h264_file = "../output/output_1080.h264"
    output_dir = "../frames"
    width = 1920
    height = 1080
    
    avg_fsnr = compute_fsnr(bgra_file, h264_file, width, height, output_dir)
    print(f"Average FSNR: {avg_fsnr:.2f} dB")
