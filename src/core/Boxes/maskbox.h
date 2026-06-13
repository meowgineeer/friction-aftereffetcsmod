#ifndef MASKBOX_H
#define MASKBOX_H

#include "Boxes/smartvectorpath.h"

class CORE_EXPORT MaskBox : public SmartVectorPath {
    e_OBJECT
    e_DECLARE_TYPE(MaskBox)
public:
    MaskBox();
    
    // We override setupCanvasMenu so it doesn't show fill/stroke options that don't apply to masks
    void setupCanvasMenu(PropertyMenu * const menu);
};

#endif // MASKBOX_H
