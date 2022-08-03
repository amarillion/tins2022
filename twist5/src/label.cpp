////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2005 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "dialog.h"
#include <math.h>
#include <stdio.h>
#include "label.h"
#include <allegro5/allegro_font.h>

Label::Label()
:Widget(),
   text(),
   alignment(0),
   valignment(0),
   wordwrap(false)

{
   SetFlag(D_AUTOSIZE);
}

/*
void Label::Setup(int x, int y, int w, int h, int key, int flags, const char *title, int a) {
   SetText(title);
   switch (a) {
      case 0:   AlignLeft();   break;
      case 1:   AlignRight();   break;
      case 2:   AlignCentre();   break;
   }
   Widget::Setup(x, y, w, h, key, flags);
}
*/

const char *Label::GetText() {
   return text.c_str();
}


int Label::GetInt() {
	return (int)atof(text.c_str());
}


double Label:: GetDouble() {
   return atof(text.c_str());
}


void Label::SetAlignment(int a) {
   alignment = a;
   UpdateSize();
}

void Label::AlignLeft() {
   SetAlignment(0);
}


void Label::AlignRight() {
   SetAlignment(1);
}


void Label::AlignCentre() {
   SetAlignment(2);
}


void Label::SetWordWrap (bool value)
{
    if (wordwrap != value)
    {
        wordwrap = value;
      UpdateSize();
    }
}


int Label::GetPreferredHeight()
{
    int result = 0;

	int type = isDisabled() ? Skin::DISABLE : Skin::NORMAL;
	ALLEGRO_FONT *f = GetFont(type);
	//TODO
//        result = f.TextHeight() * f.CountLines (text, w());
	result = 20; //TODO temporary solution

	return result;
}


void Label::doDraw(const GraphicsContext &gc) {
  int type = isDisabled() ? Skin::DISABLE : Skin::NORMAL;
  ALLEGRO_COLOR fg = theSkin->fcol[Skin::INFO_TEXT][type];
  ALLEGRO_COLOR bg = theSkin->scol[Skin::INFO_TEXT][type];
  ALLEGRO_FONT * f = GetFont(type);

  /*
   //TODO
  int textMode = GetTextMode();
  if (textMode) {
	 canvas.Clear(textMode);

  }
   */

  if (wordwrap) {
	  //TODO
//         f.BoxPrint(canvas, text, fg, bg, textMode, 0, 0, w(), h(), alignment, valignment);
	  al_draw_text (f, fg, x + gc.xofst, y + gc.yofst, alignment, text.c_str());
  }
  else {
	 int xx = 0;

	 switch (alignment) {
		case 1:      xx = getw();      break;
		case 2:      xx = getw()/2;      break;
	 };

  //TODO
//         f.GUITextout(canvas, text, xx, (h() - f.TextHeight())/2, fg, bg, textMode, alignment);
	 al_draw_text (f, fg, x + gc.xofst, y + gc.yofst, alignment, text.c_str());
  }
}

void Label::UpdateSize() {
	//TODO
	/*
   if (!text || !TestFlag(D_AUTOSIZE)) return;

   int type = Disabled() ? Skin::DISABLE : Skin::NORMAL;
   Font f = GetFont(type);
   if (wordwrap) {
      Resize(w(), GetPreferredHeight());
   }
   else {
      Resize(f.GUITextLength(text), f.TextHeight());
   }
   */
}


void Label::SetText(const char *t) {
   text = t;
   UpdateSize();
}


void Label::SetNumber(int value) {
	char buf[64];
	snprintf(buf, 64, "%d", value);
	SetText((const char *)buf);
}


void Label::SetNumber(double value) {
   char buf[256];
   snprintf(buf, 256, "%f", value);
   int l=strlen(buf)-1;
   for (int i=l; i>0; i--) {
      if (buf[i] != '0') {
         if (buf[i] != '.' && buf[i] != ',')
            buf[i+1] = 0;
         else if (i<l-1)
            buf[i+2] = 0;
         break;
      }
   }
   SetText((const char *)buf);
}


int Label::GetAlignment() {
   return alignment;
}


void Label::SetVAlignment (int value) {
   valignment = value;
}


int Label::GetVAlignment () {
   return valignment;
}


bool Label::GetWordWrap () {
   return wordwrap;
}
