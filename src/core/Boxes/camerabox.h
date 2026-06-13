#ifndef CAMERABOX_H
#define CAMERABOX_H

#include "Boxes/boundingbox.h"

class CORE_EXPORT CameraBox : public BoundingBox {
    e_OBJECT
public:
    CameraBox();
    
    SkM44 getCameraMatrix(const qreal relFrame) const;
    
    virtual qsptr<BoxRenderData> createRenderData() override {
        return nullptr; // Camera doesn't render itself
    }
    virtual HardwareSupport hardwareSupport() const override {
        return HardwareSupport::cpuOnly;
    }
};

#endif // CAMERABOX_H
