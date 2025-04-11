#include "iostream"
#include "DvsCamera.hpp"
#include "DvsCameraManager.hpp"
#include "DvsCameraUtils.hpp"
#include "EventTypes.hpp"
#include "RawEventStreamFormat.hpp"
#include "TypeUtils.hpp"
#include "fstream"
#include "thread"
#include "chrono"
#include "opencv2/opencv.hpp"
#include "atomic"
#include "mutex"

// 配置相机的事件批次参数
void configureBatchSettings(dvsense::CameraDevice& camera) {
    camera->setBatchEventsTime(10 * 1000 * 1000); // 总批次时间为 10 秒
    camera->setBatchEventsNum(1000000);          // 总批次 1,000,000 个事件
    std::cout << "[Debug] Batch settings applied: 10 seconds, 1,000,000 events" << std::endl;
}

// 动态生成唯一文件名
std::string generateUniqueFileName(const std::string& baseName, const std::string& extension) {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_time_t);

    std::ostringstream oss;
    oss << baseName << "_"
        << std::put_time(&now_tm, "%Y%m%d_%H%M%S") // 格式化时间
        << extension;

    return oss.str();
}

// 实时事件可视化（异步模式）
void visualizeEventsAsync(dvsense::CameraDevice& camera, uint16_t width, uint16_t height, std::atomic<bool>& running) {
	cv::Mat frame = cv::Mat::zeros(height, width, CV_8UC3);  // 创建空白帧
    const cv::Vec3b color_on(255, 255, 255);   // 极性为正（白色）
    const cv::Vec3b color_off(255, 0, 0);     // 极性为负（蓝色）

    std::mutex frameMutex;

    // 注册回调
    auto callback_id = camera->addEventsStreamHandleCallback([&frame, &frameMutex, color_on, color_off](const dvsense::Event2D* begin, const dvsense::Event2D* end) {
        std::lock_guard<std::mutex> lock(frameMutex);
        for (auto it = begin; it != end; ++it) {
            int x = static_cast<int>(it->x);
            int y = static_cast<int>(it->y);
            if (x >= 0 && x < frame.cols && y >= 0 && y < frame.rows) {
                frame.at<cv::Vec3b>(y, x) = it->polarity ? color_on : color_off;
            }
        }
        });

    // 主循环：实时显示事件
    while (running.load()) {
        {
            std::lock_guard<std::mutex> lock(frameMutex);
            cv::imshow("Real-Time Event Visualization", frame);
        }

        // 清除帧（可选，根据需要保留历史事件）
        frame.setTo(cv::Scalar(0, 0, 0));

        // 每帧显示约 30 FPS
        if (cv::waitKey(33) == 27) {
            running.store(false);
        }
    }

    // 移除回调并关闭窗口
    camera->removeEventsStreamHandleCallback(callback_id);
    cv::destroyWindow("Real-Time Event Visualization");
    std::cout << "[Info] Callback removed and visualization window closed." << std::endl;
}

// 累积事件数据（同步模式）
void accumulateEventsSync(dvsense::CameraDevice& camera, std::atomic<bool>& running) {
    dvsense::Event2DVector eventBatch;
    uint64_t totalTime = 0;

    while (running.load()) {
        if (camera->getNextBatch(eventBatch)) {
            uint64_t batchTime = eventBatch.back().timestamp - eventBatch.front().timestamp;
            totalTime += batchTime;
            std::cout << "[Batch] Batch Time: " << batchTime / 1e6 << " seconds"
                << ", Accumulated Time: " << totalTime / 1e6 << " seconds" << std::endl;
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // 停止条件：累积时间达到 10 秒
        if (totalTime >= 10 * 1e6) {
            running.store(false);
        }
    }
}

// 启动相机并进行操作
void startCameraStreaming(dvsense::CameraDevice& camera) {
    // 配置批次设置
    configureBatchSettings(camera);

    // 动态生成文件名
    std::string outFilePath = generateUniqueFileName("C:\\Users\\Lenovo\\Desktop\\get_File(.raw)\\camera_recording", ".raw");
    camera->startRecording(outFilePath);
    std::cout << "[Info] Recording started: " << outFilePath << std::endl;

    // 开始流式传输
    camera->start();
    std::cout << "[Info] Streaming started." << std::endl;

    // 获取相机分辨率
    uint16_t width = camera->getWidth();
    uint16_t height = camera->getHeight();
    if (width == 0 || height == 0) {
        width = 1280;
        height = 720;
        std::cout << "[Warn] Invalid resolution detected. Using default 1280x720." << std::endl;
    }

    std::atomic<bool> running{ true };

    // 启动异步事件可视化线程
    std::thread asyncVisualizationThread(visualizeEventsAsync, std::ref(camera), width, height, std::ref(running));

    // 使用同步模式累积事件
    accumulateEventsSync(camera, running);

    // 等待异步线程完成
    asyncVisualizationThread.join();

    // 停止流式传输
    camera->stop();
    std::cout << "[Info] Streaming stopped." << std::endl;

    // 停止录制
    camera->stopRecording();
    std::cout << "[Info] Recording stopped." << std::endl;
}

// 主函数
int main() {
    try {
        dvsense::DvsCameraManager cameraManager;

        // 更新相机列表
        int cameraCount = cameraManager.updateCameras();
        if (cameraCount == 0) {
            throw std::runtime_error("No cameras found.");
        }

        // 打开相机
        dvsense::CameraDevice camera = cameraManager.openCamera(cameraManager.getCameraDescs()[0].serial);
        std::cout << "[Info] Camera opened." << std::endl;

        // 启动相机并进行流式传输
        startCameraStreaming(camera);
    }
    catch (const std::exception& e) {
        std::cerr << "[Error] Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}