#include "iostream"
#include "DvsCamera.hpp"  // 相机控制
#include "DvsCameraManager.hpp" // 相机控制
#include "DvsCameraUtils.hpp"
#include "DvsFileReader.h"  // 文件操作
#include "EventTypes.hpp"  // 事件处理
#include "RawEventStreamFormat.hpp"
#include "TypeUtils.hpp"
#include "fstream"
#include "opencv2/opencv.hpp"  // OpenCV用于可视化
#include "memory"
#include "filesystem"
#include "cstdint"

// 根据原始文件名生成唯一的视频文件名
std::string generateVideoFileName(const std::string& rawFilePath) {
    // 提取原始文件名（不包括路径和扩展名）
    std::string baseName = std::filesystem::path(rawFilePath).stem().string();

    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_time_t);

    // 格式化时间为 YYYYMMDD_HHMMSS
    std::ostringstream timeStream;
    timeStream << std::put_time(&now_tm, "%Y%m%d_%H%M%S");

    // 指定保存目录
    std::string outputDirectory = "C:\\Users\\Lenovo\\Desktop\\get_File(.avi)\\";

    // 确保目录存在
    std::filesystem::create_directories(outputDirectory);

    // 生成文件名
    return outputDirectory + baseName + "_" + timeStream.str() + ".avi";
}

int main() {
    try {
        std::string filePath = "C:\\Users\\Lenovo\\Desktop\\get_File(.raw)\\camera_recording_20241214_140927.raw";

        // 创建文件读取器
        auto reader = dvsense::DvsFileReader::createFileReader(filePath);
        if (!reader) {
            std::cerr << "[Error] Failed to create file reader." << std::endl;
            return -1;
        }

        std::cout << "[Info] File reader created successfully." << std::endl;

        // 加载文件
        if (!reader->loadRawFile()) {
            std::cerr << "[Error] Failed to load file. File may be incompatible or corrupted." << std::endl;
            return -1;
        }

        // 检查时间戳
        dvsense::TimeStamp startTimestamp, endTimestamp;
        if (reader->getStartTimeStamp(startTimestamp)) {
            std::cout << "[Debug] Start timestamp: " << startTimestamp << std::endl;
        }
        else {
            std::cerr << "[Warn] Failed to get start timestamp. File might lack timestamp metadata." << std::endl;
        }

        if (reader->getEndTimeStamp(endTimestamp)) {
            std::cout << "[Debug] End timestamp: " << endTimestamp << std::endl;
        }
        else {
            std::cerr << "[Warn] Failed to get end timestamp. File might lack timestamp metadata." << std::endl;
        }

        // 获取分辨率
        uint16_t width = reader->getWidth();
        uint16_t height = reader->getHeight();
        if (width == 0 || height == 0) {
            std::cerr << "[Warn] Invalid resolution detected. Using default 1280x720." << std::endl;
            width = 1280;
            height = 720;
        }
        std::cout << "[Debug] Resolution: " << width << "x" << height << std::endl;

        // 动态生成视频文件名
        std::string outputVideo = generateVideoFileName(filePath);
        std::cout << "[Info] Output video file: " << outputVideo << std::endl;

        // 创建视频写入器
        int fps = 30;  // 设置帧率
        cv::VideoWriter videoWriter(outputVideo, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(width, height));

        if (!videoWriter.isOpened()) {
            std::cerr << "[Error] Failed to open video writer." << std::endl;
            return -1;
        }

        // 处理事件数据
        uint64_t eventsPerFrame = 1000;  // 每帧处理的事件数量
        dvsense::TimeStamp frameInterval = (endTimestamp - startTimestamp) / (fps * 10);  // 计算帧间时间间隔

        for (dvsense::TimeStamp currentTime = startTimestamp; currentTime < endTimestamp; currentTime += frameInterval) {
            // 获取当前帧时间段内的事件
            auto events = reader->getNTimeEvents(currentTime, frameInterval);
            if (!events || events->empty()) {
                continue;  // 如果没有事件，跳过
            }

            // 创建黑色帧
            cv::Mat frame = cv::Mat::zeros(height, width, CV_8UC3);

            // 绘制事件
            for (const auto& event : *events) {
                int x = static_cast<int>(event.x);
                int y = static_cast<int>(event.y);
                bool polarity = event.polarity;

                // 根据极性选择颜色
                cv::Vec3b color = polarity ? cv::Vec3b(255, 255, 255) : cv::Vec3b(255, 0, 0);  // 白色和蓝色

                // 设置像素颜色
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    frame.at<cv::Vec3b>(y, x) = color;
                }
            }

            // 写入帧到视频
            videoWriter.write(frame);
        }

        videoWriter.release();  // 释放视频写入器
        std::cout << "[Info] Video saved successfully at " << outputVideo << std::endl;
    }

    //捕获异常
    catch (const std::exception& e) {
        std::cerr << "[Error] Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
