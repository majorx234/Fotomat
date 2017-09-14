#ifndef CAMERAINTERFACE
#define CAMERAINTERFACE

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2.h>
#include <memory>
#include <QMap>
#include <QPair>
#include <QtGui/QImage>
#include <QMutex>

class CameraInterface {
   public:
    CameraInterface();
    virtual ~CameraInterface();

    std::shared_ptr<QImage> captureLiveViewImage();
    std::shared_ptr<QImage> captureImage();

    bool ok();

   protected:
   private:
    Camera* _camera;
    GPContext* _context;

    bool _initSuccessful;
    bool _errorOccured;

    static void gphoto2_ctx_msg_func(GPContext*, const char* msg, void*);
    static void gphoto2_ctx_status_func(GPContext*, const char* msg, void*);
    static void gphoto2_ctx_error_func(GPContext*, const char* msg, void*);
    static void gphoto2_ctx_idle_func(GPContext* ctx, void* data);

    static unsigned int gphoto2_ctx_progress_start_func(GPContext* context, float target, const char* text, void* data);
    static void gphoto2_ctx_progress_update_func(GPContext* context, unsigned int id, float current, void* data);
    static void gphoto2_ctx_progress_stop_func(GPContext* context, unsigned int id, void* data);

    QMap<int, QPair<float, QString>> _progressMessages;
    int _maxProgressIndex;

    CameraInterface(const CameraInterface&);
    CameraInterface& operator=(const CameraInterface&);

    QMutex _camMutex;
};

#endif
