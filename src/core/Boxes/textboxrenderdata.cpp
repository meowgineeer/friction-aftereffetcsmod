/*
#
# Friction - https://friction.graphics
#
# Copyright (c) Ole-André Rodlie and contributors
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# See 'README.md' for more information.
#
*/

// Fork of enve - Copyright (C) 2016-2020 Maurycy Liebner

#include "textboxrenderdata.h"
#include "textbox.h"
#include "PathEffects/patheffectstask.h"
#include "canvas.h"
#include "modules/skshaper/include/SkShaper.h"

qreal textLineX(const Qt::Alignment &alignment,
                const qreal lineWidth,
                const qreal maxWidth) {
    if(alignment == Qt::AlignCenter) {
        return (maxWidth - lineWidth)*0.5;
    } else if(alignment == Qt::AlignLeft) {
        return 0;
    } else {// if(alignment == Qt::AlignRight) {
        return maxWidth - lineWidth;
    }
}

qreal horizontalAdvance(const SkFont& font, const QString& str) {
    const SkScalar result = font.measureText(str.utf16(),
                                             str.size()*sizeof(short),
                                             SkTextEncoding::kUTF16);
    return static_cast<qreal>(result);
}

qreal horizontalAdvance(const SkFont& font, const QString& str,
                        const qreal letterSpacing) {
    SkScalar result = font.measureText(str.utf16(),
                                       str.size()*sizeof(short),
                                       SkTextEncoding::kUTF16);
    const qreal fontSize = static_cast<qreal>(font.getSize());
    result += static_cast<SkScalar>(fontSize*letterSpacing*str.length());
    return static_cast<qreal>(result);
}

qreal shapedHorizontalAdvance(const SkFont& font, const QString& str,
                              const qreal letterSpacing, const qreal wordSpacing,
                              const bool isRTL, SkShaper* shaper) {
    if (str.isEmpty()) return 0;
    
    class WidthRunHandler : public SkShaper::RunHandler {
    public:
        SkScalar fTotalAdvance = 0;
        void beginLine() override {}
        void runInfo(const RunInfo&) override {}
        void commitRunInfo() override {}
        Buffer runBuffer(const RunInfo& info) override { return {nullptr, nullptr, nullptr, nullptr, {0,0}}; }
        void commitRunBuffer(const RunInfo& info) override {
            fTotalAdvance += info.fAdvance.fX;
        }
        void commitLine() override {}
    };

    QByteArray utf8 = str.toUtf8();
    WidthRunHandler runHandler;
    shaper->shape(utf8.constData(), utf8.size(), font, !isRTL, SK_ScalarMax, &runHandler);

    qreal result = runHandler.fTotalAdvance;
    const qreal fontSize = static_cast<qreal>(font.getSize());
    const int nSpaces = str.count(" ");
    if(nSpaces > 0) {
        const char spaceChar = ' ';
        const SkScalar space = font.measureText(&spaceChar, sizeof(char), SkTextEncoding::kUTF8);
        result += nSpaces*space*static_cast<SkScalar>(wordSpacing - 1);
    }
    const int nNonSpaces = str.length() - nSpaces;
    result += static_cast<SkScalar>(fontSize*letterSpacing*nNonSpaces);
    return result;
}

LetterRenderData::LetterRenderData(TextBox * const parent) :
    PathBoxRenderData(parent) {
    fParentIsTarget = false;
}

void LetterRenderData::afterQued() {
    if(!fPathEffects.isEmpty() || !fFillEffects.isEmpty() ||
       !fOutlineBaseEffects.isEmpty() || !fOutlineEffects.isEmpty()) {
        const auto pathTask = enve::make_shared<PathEffectsTask>(
                    this, std::move(fPathEffects), std::move(fFillEffects),
                    std::move(fOutlineBaseEffects), std::move(fOutlineEffects));
        pathTask->addDependent(this);
        this->delayDataSet();
        pathTask->queTask();
    }
    BoxRenderData::afterQued();
}

void LetterRenderData::initialize(const qreal relFrame,
                                  const QPointF &pos,
                                  const SkGlyphID glyph,
                                  const SkFont &font,
                                  TextBox * const parent,
                                  Canvas * const scene) {
    fOriginalPos = pos;
    fLetterPos = pos;
    const auto parentM = parent->getInheritedTransformAtFrame(relFrame);
    parent->BoundingBox::setupWithoutRasterEffects(
                relFrame, parentM, this, scene);
    parent->setupPaintSettings(this, relFrame);
    parent->setupStrokerSettings(this, relFrame);
    SkPath textPath;
    font.getPath(glyph, &textPath);
    textPath.offset(pos.x(), pos.y());

    fPath = textPath;
    fEditPath = textPath;
    fFillPath = textPath;
    fOutlineBasePath = textPath;
    fStroker.strokePath(fOutlineBasePath, &fOutlinePath);

    parent->addPathEffects(relFrame, scene, fPathEffects, fFillEffects,
                           fOutlineBaseEffects, fOutlineEffects);

}

void LetterRenderData::applyTransform(const QMatrix &transform) {
    fRelTransform = transform*fRelTransform;
    fTotalTransform = fRelTransform*fInheritedTransform;
    fLetterPos = transform.map(fLetterPos);
}

WordRenderData::WordRenderData(TextBox * const parent) :
    ContainerBoxRenderData(parent) {
    fParentIsTarget = false;
}

void WordRenderData::initialize(const qreal relFrame,
                                const QPointF &pos,
                                const SkFont &font,
                                const qreal letterSpacing,
                                TextBox * const parent,
                                Canvas * const scene) {
    fOriginalPos = pos;
    fWordPos = pos;
    const auto parentM = parent->getInheritedTransformAtFrame(relFrame);
    parent->BoundingBox::setupRenderData(relFrame, parentM, this, scene);
}

void WordRenderData::applyTransform(const QMatrix &transform) {
    fRelTransform = transform*fRelTransform;
    fTotalTransform = fRelTransform*fInheritedTransform;
    fWordPos = transform.map(fWordPos);

    for(const auto& letter : fLetters) {
        letter->applyTransform(transform);
    }
}

void WordRenderData::queAllLetters() {
    for(const auto& letter : fLetters) {
        letter->queTask();
        letter->addDependent(this);
    }
}

LineRenderData::LineRenderData(TextBox * const parent) :
    ContainerBoxRenderData(parent) {
    fParentIsTarget = false;
}

void LineRenderData::initialize(const qreal relFrame,
                                const QPointF &pos,
                                const QString &line,
                                const SkFont &font,
                                const qreal letterSpacing,
                                const qreal wordSpacing,
                                const bool isRTL,
                                TextBox * const parent,
                                Canvas * const scene) {
    fOriginalPos = pos;
    fLinePos = pos;
    fString = line;
    const auto parentM = parent->getInheritedTransformAtFrame(relFrame);
    parent->BoundingBox::setupRenderData(relFrame, parentM, this, scene);

    std::unique_ptr<SkShaper> shaper = SkShaper::Make();
    if (!shaper) shaper = SkShaper::MakePrimitive();

    if (line.isEmpty()) return;

    class LineRunHandler : public SkShaper::RunHandler {
        qreal fLetterSpacing;
        qreal fWordSpacing;
        SkFont fFont;
        QPointF fStartPos;
        qreal fCurrentX;
        
        TextBox* fParent;
        LineRenderData* fLineData;
        qreal fRelFrame;
        Canvas* fScene;

        stdsptr<WordRenderData> fCurrentWord;

    public:
        std::vector<SkGlyphID> fGlyphs;
        std::vector<SkPoint> fPositions;
        std::vector<uint32_t> fClusters;
        QByteArray fUtf8;

        LineRunHandler(qreal ls, qreal ws, const SkFont& font, QPointF startPos, TextBox* parent, LineRenderData* lineData, qreal relFrame, Canvas* scene, const QByteArray& utf8)
            : fLetterSpacing(ls), fWordSpacing(ws), fFont(font), fStartPos(startPos), fCurrentX(startPos.x()), fParent(parent), fLineData(lineData), fRelFrame(relFrame), fScene(scene), fUtf8(utf8) {}

        void beginLine() override {}
        void runInfo(const RunInfo&) override {}
        void commitRunInfo() override {}

        Buffer runBuffer(const RunInfo& info) override {
            fGlyphs.resize(info.glyphCount);
            fPositions.resize(info.glyphCount);
            fClusters.resize(info.glyphCount);
            return {fGlyphs.data(), fPositions.data(), nullptr, fClusters.data(), {0, 0}};
        }

        void commitRunBuffer(const RunInfo& info) override {
            const qreal fontSize = static_cast<qreal>(fFont.getSize());
            const qreal lsOffset = fontSize * fLetterSpacing;
            
            const char spaceChar = ' ';
            const SkScalar spaceAdvance = fFont.measureText(&spaceChar, sizeof(char), SkTextEncoding::kUTF8);

            for (size_t i = 0; i < info.glyphCount; ++i) {
                // Determine if this is a space based on the utf8 cluster
                uint32_t cluster = fClusters[i];
                bool isSpace = (cluster < fUtf8.size() && fUtf8[cluster] == ' ');

                if (!fCurrentWord || isSpace) {
                    fCurrentWord = enve::make_shared<WordRenderData>(fParent);
                    fCurrentWord->initialize(fRelFrame, QPointF(fCurrentX, fStartPos.y()), fFont, fLetterSpacing, fParent, fScene);
                    fLineData->fWords << fCurrentWord;
                    fLineData->fChildrenRenderData << fCurrentWord;
                }

                if (!isSpace) {
                    const auto letter = enve::make_shared<LetterRenderData>(fParent);
                    letter->initialize(fRelFrame, QPointF(fCurrentX + fPositions[i].fX, fStartPos.y()), fGlyphs[i], fFont, fParent, fScene);
                    fCurrentWord->fLetters << letter;
                    fCurrentWord->fChildrenRenderData << letter;
                }

                qreal advanceX = 0;
                if (i + 1 < info.glyphCount) {
                    advanceX = fPositions[i+1].fX - fPositions[i].fX;
                } else {
                    advanceX = info.fAdvance.fX - fPositions[i].fX;
                }

                if (isSpace) {
                    fCurrentX += advanceX + (spaceAdvance * (fWordSpacing - 1.0));
                } else {
                    fCurrentX += advanceX + lsOffset;
                }
            }
        }
        void commitLine() override {}
    };

    QByteArray utf8 = line.toUtf8();
    LineRunHandler runHandler(letterSpacing, wordSpacing, font, pos, parent, this, relFrame, scene, utf8);
    shaper->shape(utf8.constData(), utf8.size(), font, !isRTL, SK_ScalarMax, &runHandler);
}

void LineRenderData::applyTransform(const QMatrix &transform) {
    fRelTransform = transform*fRelTransform;
    fTotalTransform = fRelTransform*fInheritedTransform;
    fLinePos = transform.map(fLinePos);

    for(const auto& word : fWords) {
        word->applyTransform(transform);
    }
}

void LineRenderData::queAllWords() {
    for(const auto& word : fWords) {
        word->queAllLetters();
        word->queTask();
        word->addDependent(this);
    }
}

TextBoxRenderData::TextBoxRenderData(TextBox* const parent) :
    ContainerBoxRenderData(parent) {}

void TextBoxRenderData::initialize(const QString &text,
                                   const SkFont &font,
                                   const qreal letterSpacing,
                                   const qreal wordSpacing,
                                   const qreal lineSpacing,
                                   const Qt::Alignment hAlignment,
                                   const Qt::Alignment vAlignment,
                                   const bool isRTL,
                                   TextBox * const parent,
                                   Canvas* const scene) {
    const QStringList lines = text.split(QRegExp("\n|\r\n|\r"));

    qreal maxWidth = 0;

    std::unique_ptr<SkShaper> shaper = SkShaper::Make();
    if (!shaper) shaper = SkShaper::MakePrimitive();

    for(const auto& line : lines) {
        const qreal lineWidth = shapedHorizontalAdvance(font, line, letterSpacing,
                                                        wordSpacing, isRTL, shaper.get());
        if(lineWidth > maxWidth) maxWidth = lineWidth;
    }

    const qreal lineInc = static_cast<qreal>(font.getSpacing())*lineSpacing;

    qreal xTranslate;
    if(hAlignment == Qt::AlignLeft) xTranslate = 0;
    else if(hAlignment == Qt::AlignRight) xTranslate = -maxWidth;
    else /*if(hAlignment == Qt::AlignCenter)*/ xTranslate = -0.5*maxWidth;

    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    const qreal height = (lines.count() - 1)*lineInc +
            static_cast<qreal>(metrics.fAscent + metrics.fDescent);
    qreal yTranslate;
    if(vAlignment == Qt::AlignTop) yTranslate = 0;
    else if(vAlignment == Qt::AlignBottom) yTranslate = -height;
    else /*if(vAlignment == Qt::AlignCenter)*/ yTranslate = -0.5*height;

    qreal yPos = yTranslate;
    for(const auto& lineStr : lines) {
        const qreal lineWidth = shapedHorizontalAdvance(font, lineStr, letterSpacing,
                                                        wordSpacing, isRTL, shaper.get());
        const qreal xPos = textLineX(hAlignment, lineWidth, maxWidth) + xTranslate;
        const auto line = enve::make_shared<LineRenderData>(parent);
        line->initialize(fRelFrame, QPointF(xPos, yPos), lineStr,
                         font, letterSpacing, wordSpacing, isRTL, parent, scene);
        fLines << line;
        fChildrenRenderData << line;
        yPos += lineInc;
    }
}

void TextBoxRenderData::queAllLines() {
    for(const auto& line : fLines) {
        line->queAllWords();
        line->queTask();
        line->addDependent(this);
    }
}
