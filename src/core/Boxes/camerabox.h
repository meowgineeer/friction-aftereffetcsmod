#ifndef CAMERABOX_H
#define CAMERABOX_H

#include "Boxes/boundingbox.h"

class CORE_EXPORT CameraBox : public BoundingBox {
    e_OBJECT
public:
    CameraBox();
    ~CameraBox() override;

    
    SkM44 getCameraMatrix(const qreal relFrame) const;
    
    stdsptr<BoxRenderData> createRenderData() override;
    virtual HardwareSupport hardwareSupport() const override {
        return HardwareSupport::cpuOnly;
    }
};

#endif // CAMERABOX_H
