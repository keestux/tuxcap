#include "HyperlinkWidget.h"
#include "Graphics.h"
#include "ImageFont.h"
#if 0
#include "SysFont.h"
#endif
#include "WidgetManager.h"

using namespace Sexy;

HyperlinkWidget::HyperlinkWidget(int theId, ButtonListener* theButtonListener) :
    ButtonWidget(theId, theButtonListener),
    mColor(255, 255, 255),
    mOverColor(255, 255, 255)
{
    mDoFinger = true;
    mUnderlineOffset = 3;
    mUnderlineSize = 1;
}

void HyperlinkWidget::Draw(Graphics* g)
{
#if 0
    if (mFont == NULL)
        mFont = new SysFont(mWidgetManager->mApp, "Arial Unicode MS", 10); //baz changed
#endif

    int aFontX = (mWidth - mFont->StringWidth(mLabel))/2;
    int aFontY = (mHeight + mFont->GetAscent())/2 - 1;

    if (mIsOver)
        g->SetColor(mOverColor);
    else
        g->SetColor(mColor);

    g->SetFont(mFont);
    g->DrawString(mLabel, aFontX, aFontY);

    for (int i = 0; i < mUnderlineSize; i++)
        g->FillRect(aFontX, aFontY+mUnderlineOffset+i, mFont->StringWidth(mLabel), 1);
}

void HyperlinkWidget::MouseEnter()
{
    ButtonWidget::MouseEnter();

    MarkDirtyFull();
}

void HyperlinkWidget::MouseLeave()
{
    ButtonWidget::MouseLeave();

    MarkDirtyFull();
}

