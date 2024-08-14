def compare_bgra_files(file1, file2):
    try:
        with open(file1, 'rb') as f1, open(file2, 'rb') as f2:
            while True:
                # 逐块读取数据进行比较，块大小可以调节
                chunk1 = f1.read(4096)
                chunk2 = f2.read(4096)

                if chunk1 != chunk2:
                    return False
                
                # 如果两个文件都达到末尾
                if not chunk1:
                    break
        return True
    except FileNotFoundError as e:
        print(f"Error: {e}")
        return False

if __name__ == "__main__":
    file1 = "../input/input_1080.bgra"
    file2 = "../output/output_1080.bgra"
    
    if compare_bgra_files(file1, file2):
        print("The two BGRA files are identical.")
    else:
        print("The two BGRA files are different.")
