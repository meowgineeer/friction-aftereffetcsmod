#include "maskbox.h"

e_DEFINE_TYPE(MaskBox)

MaskBox::MaskBox() :
    SmartVectorPath()
{
    mType = eBoxType::mask;
    setName("Mask");
}

void MaskBox::setupCanvasMenu(PropertyMenu * const menu) {
    if (menu->hasActionsForType<MaskBox>()) { return; }
    menu->addedActionsForType<MaskBox>();
    SmartVectorPath::setupCanvasMenu(menu);
}
