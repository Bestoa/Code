#!/usr/bin/env python3
"""
生成黑白棋盘格纹理
"""

import numpy as np
from PIL import Image

def generate_checkerboard(width=512, height=512, tile_size=64):
    """
    生成黑白棋盘格纹理
    
    参数:
        width: 纹理宽度
        height: 纹理高度  
        tile_size: 每个格子的像素大小
    """
    # 创建空图像数组
    image = np.zeros((height, width, 3), dtype=np.uint8)
    
    # 生成棋盘格图案
    for y in range(height):
        for x in range(width):
            # 计算当前像素属于哪个格子
            tile_x = x // tile_size
            tile_y = y // tile_size
            
            # 判断黑白格：如果(tile_x + tile_y)是偶数则为白色，奇数则为黑色
            if (tile_x + tile_y) % 2 == 0:
                # 白色
                image[y, x] = [255, 255, 255]
            else:
                # 黑色
                image[y, x] = [0, 0, 0]
    
    return image

def save_checkerboard(filename="checkerboard.png", **kwargs):
    """生成并保存棋盘格纹理"""
    image_array = generate_checkerboard(**kwargs)
    image = Image.fromarray(image_array, 'RGB')
    image.save(filename)
    print(f"棋盘格纹理已保存到: {filename}")
    print(f"尺寸: {image.width}x{image.height}")
    print(f"格式: PNG")
    
    return image

if __name__ == "__main__":
    # 生成不同尺寸的棋盘格纹理
    textures = [
        ("checkerboard_64x64.png", 64, 64, 8),
        ("checkerboard_128x128.png", 128, 128, 16),
        ("checkerboard_256x256.png", 256, 256, 32),
        ("checkerboard_512x512.png", 512, 512, 64),
        ("checkerboard_1024x1024.png", 1024, 1024, 128),
    ]
    
    for filename, width, height, tile_size in textures:
        save_checkerboard(
            filename=filename,
            width=width,
            height=height,
            tile_size=tile_size
        )