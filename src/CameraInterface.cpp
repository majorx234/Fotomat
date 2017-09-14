#include "CameraInterface.hpp"
#include <cassert>
#include <gphoto2/gphoto2-file.h>
#include <gphoto2/gphoto2-filesys.h>
#include <gphoto2/gphoto2-result.h>
#include <QDebug>
#include <QThreadPool>

void CameraInterface::gphoto2_ctx_msg_func(GPContext*, const char* msg, void*) {
    qDebug() << "<GP2 Message> " << msg;
}

void CameraInterface::gphoto2_ctx_status_func(GPContext*, const char* msg, void*) {
    qDebug() << "<GP2 Status> " << msg;
}

void CameraInterface::gphoto2_ctx_error_func(GPContext*, const char* msg, void* data) {
    qDebug() << "<GP2 Error> " << msg;
    CameraInterface* camInterface = static_cast<CameraInterface*>(data);
    camInterface->_errorOccured = true;
}

void CameraInterface::gphoto2_ctx_idle_func(GPContext*, void*) {
    qDebug() << "<GP2 Idle>";
}

unsigned int CameraInterface::gphoto2_ctx_progress_start_func(GPContext*, float target, const char* text, void* data) {
    qDebug() << "<GP2 Progress Start> " << target << " " << text;
    CameraInterface* camInterface = static_cast<CameraInterface*>(data);
    int id = camInterface->_maxProgressIndex;
    ++camInterface->_maxProgressIndex;
    camInterface->_progressMessages.insert(id, QPair<float, QString>(target, text));
    return id;
}

void CameraInterface::gphoto2_ctx_progress_update_func(GPContext*, unsigned int id, float current, void* data) {
    CameraInterface* camInterface = static_cast<CameraInterface*>(data);
    float target = camInterface->_progressMessages[id].first;
    qDebug() << "<GP2 Progress Update> " << id << " " << current / target * 100.0f << "%";
}

void CameraInterface::gphoto2_ctx_progress_stop_func(GPContext*, unsigned int id, void* data) {
    CameraInterface* camInterface = static_cast<CameraInterface*>(data);
    qDebug() << "<GP2 Progress Stop> " << camInterface->_progressMessages[id].second.toStdString().c_str()
             << " Finished";
    auto it = camInterface->_progressMessages.find(id);
    if (it != camInterface->_progressMessages.end())
        camInterface->_progressMessages.erase(it);
}

CameraInterface::CameraInterface()
    : _camera(nullptr),
      _context(gp_context_new()),
      _initSuccessful(false),
      _errorOccured(false),
      _progressMessages(),
      _maxProgressIndex(0),
      _camMutex() {
    assert(_context != nullptr);
    assert(gp_camera_new(&_camera) == GP_OK);

    // set callbacks for camera messages
    gp_context_set_error_func(_context, gphoto2_ctx_error_func, this);
    gp_context_set_message_func(_context, gphoto2_ctx_msg_func, this);
    gp_context_set_status_func(_context, gphoto2_ctx_status_func, this);
    gp_context_set_idle_func(_context, gphoto2_ctx_idle_func, this);

    gp_context_set_progress_funcs(_context, gphoto2_ctx_progress_start_func, gphoto2_ctx_progress_update_func,
                                  gphoto2_ctx_progress_stop_func, this);

    // list available cameras
    CameraList* list = nullptr;
    gp_list_new(&list);
    gp_camera_autodetect(list, _context);
    gp_list_sort(list);
    for (int i = 0; i < gp_list_count(list); ++i) {
        const char* name = nullptr;
        const char* value = nullptr;
        gp_list_get_name(list, i, &name);
        gp_list_get_value(list, i, &value);
        qDebug() << name << " -> " << value;
    }
    gp_list_free(list);

    int ret = gp_camera_init(_camera, _context);
    if (ret != GP_OK) {
        qDebug() << "<GP2 Init Error>" << gp_result_as_string(ret);
        gp_camera_free(_camera);
        _camera = nullptr;
        _initSuccessful = false;
    } else {
        _initSuccessful = true;
    }
}

CameraInterface::~CameraInterface() {
    if (_camera != nullptr) {
        gp_camera_exit(_camera, _context);
        gp_camera_unref(_camera);
        _camera = nullptr;
    }
    if (_context != nullptr) {
        gp_context_unref(_context);
        _context = nullptr;
    }
}

std::shared_ptr<QImage> CameraInterface::captureLiveViewImage() {
    QMutexLocker mLocker(&_camMutex);

    if (!_initSuccessful || _camera == nullptr || _context == nullptr) {
        qDebug() << "Capture after unsucessful initialization";
        return std::shared_ptr<QImage>(nullptr);
    }

    CameraFile* file = nullptr;
    assert(gp_file_new(&file) == GP_OK);
    int ret = gp_camera_capture_preview(_camera, file, _context);
    if (ret != GP_OK) {
        return std::shared_ptr<QImage>(nullptr);
    }

    // create image
    const unsigned char* data;
    unsigned long size;
    assert(gp_file_get_data_and_size(file, (const char**)&data, &size) == GP_OK);
    std::shared_ptr<QImage> vidImg(new QImage);
    vidImg->loadFromData(data, size, "JPG");

    assert(gp_file_unref(file) == GP_OK);

    return vidImg;
}

std::shared_ptr<QImage> CameraInterface::captureImage() {
    QMutexLocker mLocker(&_camMutex);

    if (!_initSuccessful || _camera == nullptr || _context == nullptr) {
        qDebug() << "Capture after unsucessful initialization";
        return std::shared_ptr<QImage>(nullptr);
    }

    CameraStorageInformation* storageInfo;
    int numberOfRowsInfo;
    int storageInfoRet = gp_camera_get_storageinfo(_camera, &storageInfo, &numberOfRowsInfo, _context);
    assert(storageInfoRet == GP_OK);

    for (int i = 0; i < numberOfRowsInfo; ++i) {
        qDebug() << storageInfo[i].freekbytes << "KB / " << storageInfo[i].capacitykbytes << " KB";
    }

    free(storageInfo);

    CameraFilePath filepath;
    memset(&filepath, 0, sizeof(CameraFilePath));
    assert(gp_camera_capture(_camera, GP_CAPTURE_IMAGE, &filepath, _context) == GP_OK);

    CameraFile* imageFile = nullptr;
    assert(gp_file_new(&imageFile) == GP_OK);
    assert(gp_camera_file_get(_camera, filepath.folder, filepath.name, GP_FILE_TYPE_NORMAL, imageFile, _context) ==
           GP_OK);
    assert(gp_camera_file_delete(_camera, filepath.folder, filepath.name, _context) == GP_OK);

    // create image
    const unsigned char* data;
    unsigned long size;
    assert(gp_file_get_data_and_size(imageFile, (const char**)&data, &size) == GP_OK);
    std::shared_ptr<QImage> stillImg(new QImage);
    stillImg->loadFromData(data, size, "JPG");

    assert(gp_file_unref(imageFile) == GP_OK);

    return stillImg;
}

bool CameraInterface::ok() {
    return _initSuccessful && !_errorOccured;
}
