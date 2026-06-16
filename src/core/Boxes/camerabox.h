#ifndef CAMERABOX_H
#define CAMERABOX_H

#include "Boxes/boundingbox.h"

class CORE_EXPORT CameraBox : public BoundingBox {
    e_OBJECT
    e_DECLARE_TYPE(CameraBox)
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
