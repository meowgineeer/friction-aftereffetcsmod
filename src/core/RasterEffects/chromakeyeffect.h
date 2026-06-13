#ifndef CHROMAKEYEFFECT_H
#define CHROMAKEYEFFECT_H

#include "rastereffect.h"
#include "Animators/coloranimator.h"
#include "Animators/qrealanimator.h"

class ChromaKeyEffect : public RasterEffect {
public:
    ChromaKeyEffect();

    stdsptr<RasterEffectCaller> getEffectCaller(
            const qreal relFrame, const qreal resolution,
            const qreal influence, BoxRenderData * const data) const override;
private:
    qsptr<ColorAnimator> mKeyColor;
    qsptr<QrealAnimator> mTolerance;
    qsptr<QrealAnimator> mSoftness;
};

#endif // CHROMAKEYEFFECT_H
