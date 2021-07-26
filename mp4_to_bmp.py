# 将MP4转换为单片机所能读取的bmp文件
import cv2

mp4_addr = "E:\\bilibili\\JiJiDown\\Download\\demo2.mp4"
bmp_addr = "C:\\Users\\86181\\Desktop\\new5\\"
timeF = 10  #设定帧率(此处为每10帧截屏一次)
size = (160, 120)  # 要保存的图片尺寸


def save_image(image, addr, num):
    address = addr + str(num) + '.bmp'
    image = cv2.resize(image, size, interpolation=cv2.INTER_AREA)
    cv2.imwrite(address, image)


videoCapture = cv2.VideoCapture(mp4_addr)  # 读取MP4
success, frame = videoCapture.read()  # 读取帧

i = 0
j = 0
while success:
    i = i + 1
    if (i % timeF == 0):
        j = j + 1
        save_image(frame, bmp_addr, j)
        print('save image:', i)
    success, frame = videoCapture.read()
