def compare_bgra_files(file1, file2):
    diff_count = 0  
    total_bytes = 0  
    try:
        with open(file1, 'rb') as f1, open(file2, 'rb') as f2:
            offset = 0  
            while True:
                chunk1 = f1.read(4096)
                chunk2 = f2.read(4096)

                if not chunk1 and not chunk2:
                    break  

                if len(chunk1) != len(chunk2):
                    diff_count += max(len(chunk1), len(chunk2))  
                    total_bytes += max(len(chunk1), len(chunk2))  
                    break

                total_bytes += len(chunk1)  

                for i in range(len(chunk1)):
                    if chunk1[i] != chunk2[i]:
                        diff_count += 1
                        print(f"Difference at byte offset {offset + i}: {chunk1[i]} != {chunk2[i]}")
                
                offset += len(chunk1)

        if total_bytes > 0:
            diff_ratio = diff_count / total_bytes  
        else:
            diff_ratio = None  

        return diff_count, diff_ratio
    except FileNotFoundError as e:
        print(f"Error: {e}")
        return -1, None  

if __name__ == "__main__":
    file1 = "../input/input_1080.bgra"
    file2 = "../output/output_1080.bgra"
    
    diff_count, diff_ratio = compare_bgra_files(file1, file2)
    if diff_count == -1:
        print("File not found.")
    elif diff_count == 0:
        print("The two BGRA files are identical.")
    else:
        print(f"The two BGRA files have {diff_count} differences.")
        if diff_ratio is not None:
            print(f"Difference ratio: {diff_ratio:.4%}")
