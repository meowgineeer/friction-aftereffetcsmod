#include "camerabox.h"
#include "boxrenderdata.h"
#include "Animators/transformanimator.h"

CameraBox::CameraBox() : BoundingBox("Camera", eBoxType::camera) {
    // Zoom / Focal length property can be added here
}

CameraBox::~CameraBox() = default;


SkM44 CameraBox::getCameraMatrix(const qreal relFrame) const {
    if (mTransformAnimator) {
        return mTransformAnimator->getTotalTransform3DAtFrame(relFrame);
    }
    return SkM44();
}

stdsptr<BoxRenderData> CameraBox::createRenderData() {
    return nullptr;
}
