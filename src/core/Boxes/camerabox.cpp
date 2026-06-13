#include "camerabox.h"
#include "Animators/transformanimator.h"

CameraBox::CameraBox() : BoundingBox("Camera", eBoxType::camera) {
    // Zoom / Focal length property can be added here
}

SkM44 CameraBox::getCameraMatrix(const qreal relFrame) const {
    if (mTransformAnimator) {
        return mTransformAnimator->getTotalTransform3DAtFrame(relFrame);
    }
    return SkM44();
}
