#include "chromakeyeffect.h"
#include "gpurendertools.h"
#include "openglrastereffectcaller.h"

#include "Animators/coloranimator.h"
#include "Animators/qrealanimator.h"
#include "appsupport.h"
#include <cmath>

ChromaKeyEffect::ChromaKeyEffect() :
    RasterEffect("chromaKey",
                 AppSupport::getRasterEffectHardwareSupport("ChromaKey",
                                                            HardwareSupport::gpuPreffered),
                 true,
                 RasterEffectType::CHROMA_KEY)
{
    mKeyColor = enve::make_shared<ColorAnimator>(QColor(0, 255, 0), "keyColor");
    ca_addChild(mKeyColor);

    mTolerance = enve::make_shared<QrealAnimator>(0.1, 0, 1, 0.01, "tolerance");
    ca_addChild(mTolerance);

    mSoftness = enve::make_shared<QrealAnimator>(0.1, 0, 1, 0.01, "softness");
    ca_addChild(mSoftness);
}

class ChromaKeyEffectCaller : public OpenGLRasterEffectCaller {
public:
    ChromaKeyEffectCaller(const HardwareSupport hwSupport,
                          const QColor& keyColor,
                          const qreal tolerance,
                          const qreal softness) :
        OpenGLRasterEffectCaller(sInitialized, sProgramId,
                                 ":/shaders/chromakeyeffect.frag",
                                 hwSupport),
        mKeyColor(keyColor),
        mTolerance(tolerance),
        mSoftness(softness) {}

    void processCpu(CpuRenderTools& renderTools, const CpuRenderData& data) override;
protected:
    void iniVars(QGL33 * const gl) const override {
        sKeyColorU = gl->glGetUniformLocation(sProgramId, "keyColor");
        sToleranceU = gl->glGetUniformLocation(sProgramId, "tolerance");
        sSoftnessU = gl->glGetUniformLocation(sProgramId, "softness");
    }

    void setVars(QGL33 * const gl) const override {
        gl->glUseProgram(sProgramId);
        gl->glUniform3f(sKeyColorU, mKeyColor.redF(), mKeyColor.greenF(), mKeyColor.blueF());
        gl->glUniform1f(sToleranceU, mTolerance);
        gl->glUniform1f(sSoftnessU, mSoftness);
    }
private:
    static bool sInitialized;
    static GLuint sProgramId;
    static GLint sKeyColorU;
    static GLint sToleranceU;
    static GLint sSoftnessU;

    const QColor mKeyColor;
    const qreal mTolerance;
    const qreal mSoftness;
};

bool ChromaKeyEffectCaller::sInitialized = false;
GLuint ChromaKeyEffectCaller::sProgramId = 0;
GLint ChromaKeyEffectCaller::sKeyColorU = -1;
GLint ChromaKeyEffectCaller::sToleranceU = -1;
GLint ChromaKeyEffectCaller::sSoftnessU = -1;

stdsptr<RasterEffectCaller> ChromaKeyEffect::getEffectCaller(
        const qreal relFrame, const qreal resolution,
        const qreal influence, BoxRenderData * const data) const {
    Q_UNUSED(resolution)
    Q_UNUSED(data)
    Q_UNUSED(influence)

    const QColor keyColor = mKeyColor->getEffectiveValue(relFrame);
    const qreal tolerance = mTolerance->getEffectiveValue(relFrame);
    const qreal softness = mSoftness->getEffectiveValue(relFrame);

    return enve::make_shared<ChromaKeyEffectCaller>(instanceHwSupport(), keyColor, tolerance, softness);
}

void ChromaKeyEffectCaller::processCpu(CpuRenderTools& renderTools, const CpuRenderData& data) {
    const int xMin = data.fTexTile.left();
    const int xMax = data.fTexTile.right();
    const int yMin = data.fTexTile.top();
    const int yMax = data.fTexTile.bottom();

    for(int yi = yMin; yi <= yMax; yi++) {
        auto dst = static_cast<uchar*>(renderTools.fDstBtmp.getAddr(0, yi - yMin));
        auto src = static_cast<uchar*>(renderTools.fSrcBtmp.getAddr(xMin, yi));
        for(int xi = xMin; xi <= xMax; xi++) {
            const uchar texR = *src++;
            const uchar texG = *src++;
            const uchar texB = *src++;
            const uchar texA = *src++;

            if(texA == 0) {
                for(int i = 0; i < 4; i++) *dst++ = 0;
                continue;
            }

            const qreal r = texR / 255.0 / (texA / 255.0);
            const qreal g = texG / 255.0 / (texA / 255.0);
            const qreal b = texB / 255.0 / (texA / 255.0);
            
            const qreal kr = mKeyColor.redF();
            const qreal kg = mKeyColor.greenF();
            const qreal kb = mKeyColor.blueF();

            const qreal y1 = 0.299*r + 0.587*g + 0.114*b;
            const qreal u1 = 0.492*(b - y1);
            const qreal v1 = 0.877*(r - y1);
            
            const qreal ky = 0.299*kr + 0.587*kg + 0.114*kb;
            const qreal ku = 0.492*(kb - ky);
            const qreal kv = 0.877*(kr - ky);

            const qreal dist = std::sqrt((u1-ku)*(u1-ku) + (v1-kv)*(v1-kv));
            qreal blend = 0.0;
            if (mSoftness <= 0.0001) {
                blend = (dist < mTolerance) ? 0.0 : 1.0;
            } else {
                if (dist < mTolerance) blend = 0.0;
                else if (dist > mTolerance + mSoftness) blend = 1.0;
                else blend = (dist - mTolerance) / mSoftness;
                blend = blend * blend * (3.0 - 2.0 * blend); // smoothstep
            }

            *dst++ = texR * blend;
            *dst++ = texG * blend;
            *dst++ = texB * blend;
            *dst++ = texA * blend;
        }
    }
}
