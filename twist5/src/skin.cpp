////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2005 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "skin.h"
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include "color.h"

const int Skin::nBitmaps = 46;
const int Skin::nCursors = 12;
const int Skin::nFonts = 8;
const int Skin::nSamples = 7;
const int Skin::nBitmapsEx = 1024;
const int Skin::nCursorsEx = 64;
const int Skin::nFontsEx = 64;
const int Skin::nSamplesEx = 1024;

/**
 * Helper function
 * parses color from a comma-separated string
 * copied from MASkinG.
 */
ALLEGRO_COLOR parseColor(const char *str) {
   char *str2 = new char[1+strlen(str)];
   int r = 0;
   int g = 0;
   int b = 0;
   int a = 255;

   if (!strcmp(str, "-1")) {
      delete [] str2;
      return TRANSPARENT;
   }
   else {
      strcpy(str2, str);
      char *tok;

      tok = strtok(str2, ", ;");
      if (tok) {
         r = strtol(tok, NULL, 10);

         tok = strtok(0, ", ;");
         if (tok) {
            g = strtol(tok, NULL, 10);

            tok = strtok(0, ", ;");
            if (tok) {
               b = strtol(tok, NULL, 10);

               tok = strtok(0, ", ;");
               if (tok) {
                  a = strtol(tok, NULL, 10);
               }
            }
         }
      }
   }

   delete [] str2;
   return al_map_rgba(r, g, b, a);
}

static const char *bitmapName[] = { "BOX", "BOX_SHADDOW", "BUTTON", "CHECKBOX", "CLEAR_BACK",
"COMBO_BUTTON", "HSLIDER_BACK", "HSLIDER_GRIP", "ICONEXIT",
   "ICONMAX", "ICONMIN", "ICONRESTORE", "LIST", "MENU_BACK", "MENU_BUTTON",
   "MENU_ITEM", "PANEL_GROOVE", "PANEL_RAISED", "PANEL_RIDGE", "PANEL_SUNKEN",
   "PROGRESSH", "PROGRESSV", "RADIO", "SCROLL_DOWN", "SCROLL_HBACK", "SCROLL_HGRIP",
   "SCROLL_HGRIPOVERLAY", "SCROLL_LEFT", "SCROLL_RIGHT", "SCROLL_UP", "SCROLL_VBACK",
   "SCROLL_VGRIP", "SCROLL_VGRIPOVERLAY", "SEPARATORH", "SEPARATORV", "TAB_BUTTON",
   "TAB_WINDOW", "VSLIDER_BACK", "VSLIDER_GRIP", "WINBOTTOM", "WINDOW", "WINGRIP",
   "WINLEFT", "WINRIGHT", "WINTEXT", "WINTOP" };

static const char *cursorName[] = {
   "MOUSE_ALTERNATIVE",
   "MOUSE_CROSSHAIR",
   "MOUSE_HOURGLASS",
   "MOUSE_MOVE",
   "MOUSE_NORMAL",
   "MOUSE_SELECT",
   "MOUSE_SIZE_DIAGONAL1",
   "MOUSE_SIZE_DIAGONAL2",
   "MOUSE_SIZE_HORIZONTAL",
   "MOUSE_SIZE_VERTICAL",
   "MOUSE_TEXTSELECT",
   "MOUSE_UNAVAILABLE"
};

////////////////////////////////////////////////////////////////////////////////
// The default constructor: resets colors, fonts and bitmaps to default values
Skin::Skin(const char *fileName) {
   int i, j;
   lastError = Error(Error::NONE);
   skinFilePath = NULL;

   // fonts
   fntList.reserve(nFontsEx);
   for (i=0; i<nFontsEx; i++) {
      fntList.push_back(new Font);
   }

   // bitmaps
   bmpList.reserve(nBitmapsEx);
   for (i=0; i<nBitmapsEx; i++) {
      bmpList.push_back(new Bitmap);
   }

   // cursors
   curList.reserve(nCursorsEx);
   for (i=0; i<nCursorsEx; i++) {
      curList.push_back(new Cursor);
   }

   // samples
   smpList.reserve(nSamplesEx);
   for (i=0; i<nSamplesEx; i++) {
      smpList.push_back(new Sample);
   }

   // font info
   for (i=0; i<nInfoItems; i++) {
      for (j=0; j<4; j++) {
         fnt[i][j] = 0;
         fcol[i][j] = BLACK;
         scol[i][j] = MAGIC_PINK;
      }
   }

   skinFilePath = NULL;
   if (strcmp(fileName, "default") != 0 && strcmp(fileName, "") != 0) {
      assert(false && "Not Implemented");
   }
   else {
      Reset();
   }
}


////////////////////////////////////////////////////////////////////////////////
// Destructor (should unload data but that seems to crash the program)
Skin::~Skin() {
   if (skinFilePath) {
      delete [] skinFilePath;
   }

   Unload();

   int i;
   for (i=0; i<nBitmapsEx; i++) {
      delete bmpList[i];
   }
   bmpList.clear();

   for (i=0; i<nFontsEx; i++) {
      delete fntList[i];
   }
   fntList.clear();

   for (i=0; i<nCursorsEx; i++) {
      delete curList[i];
   }
   curList.clear();

   for (i=0; i<nSamplesEx; i++) {
      delete smpList[i];
   }
   smpList.clear();
}


////////////////////////////////////////////////////////////////////////////////
// A helper function for setting the skin colors to defaults
void Skin::ResetColors() {
   // Global skin colors
   c_face =   al_map_rgb(210,210,210);
   c_font =   al_map_rgb( 16, 16, 16);
   c_shad1 =   al_map_rgb(255,255,255);
   c_shad2 =   al_map_rgb( 80, 80, 80);
   c_disable =   al_map_rgb(128,128,128);
   c_select =   al_map_rgb(128,192,128);
   c_deselect =al_map_rgb(224,224,224);
   c_focus =   al_map_rgb(128,192,128);
   c_sunken =   al_map_rgb(232,232,232);
   c_back =   al_map_rgb(180,180,180);

   // Font info for widgets
   for (int i=0; i<nInfoItems; i++) {
      fcol[i][NORMAL] = c_font;
      fcol[i][SELECT] = c_font;
      fcol[i][DISABLE] = c_disable;
      fcol[i][FOCUS] = c_font;
      scol[i][NORMAL] = TRANSPARENT;
      scol[i][SELECT] = TRANSPARENT;
      scol[i][DISABLE] = c_shad1;
      scol[i][FOCUS] = TRANSPARENT;
   }

   // The hyper-text is a special case
   fcol[INFO_HYPER][NORMAL] = al_map_rgb(0,0,128);
   fcol[INFO_HYPER][SELECT] =  al_map_rgb(128,0,128);
   fcol[INFO_HYPER][FOCUS] =  al_map_rgb(128,0,128);
}


////////////////////////////////////////////////////////////////////////////////
// Supposed to unload the data
void Skin::Unload() {
   int i;

   for (i=0; i<nBitmapsEx; i++) {
      bmpList[i]->Destroy();
   }

   for (i=0; i<nCursorsEx; i++) {
      curList[i]->Destroy();
   }

   for (i=0; i<nSamplesEx; i++) {
      smpList[i]->Destroy();
   }

   for (i=0; i<nFontsEx; i++) {
      fntList[i]->Destroy();
   }
}


////////////////////////////////////////////////////////////////////////////////
// Draws a default skin bitmap
void Skin::GenerateDefaultBitmap(int i) {
   Bitmap *bmp = bmpList[i];
   static int w[] =  { 64,64,18,26,72,18,18,11,15,15,15,15,36,64,18, 48,64,64,64,64,12,36,26,16,18,18,16,16,16,16,16,16,16,12, 2,18,64,88,88,30,30,30, 8, 8,12,30 };
   static int h[] =  { 64,64,72,52,72,80,88,88,56,56,56,56,96,64,24,112,64,64,64,64,36,12,52,64,64,64,64,64,64,64,64,48,64, 2,12,72,64,18,11, 8,30,36,72,72,17, 8 };
   static int tw[] = { 60,58,14,13,68,18,14,11,15,15,15,15,32,60,18, 16,60,60,60,60, 8,14,13,16,14,14,12,16,16,16,12,12,12, 8, 2,14,60,18,22,-1,-1,-1,-1,-1,-1,-1 };
   static int th[] = { 48,58,14,13,68,20,18,22,14,14,14,14, 8,60,12, 16,60,60,60,60,14, 8,13,16,12,12,12,16,16,16,12, 8,12, 2, 8,14,60,14,11,-1,-1,-1,-1,-1,-1,-1 };

   bmp->Create(w[i], h[i], MASSettings::useVideoMemory ? Bitmap::VIDEO : Bitmap::MEMORY);
   bmp->ThickX(tw[i]);
   bmp->ThickY(th[i]);
   bmp->Clear(c_face);

   switch (i) {
      case BOX:
         bmp->Rectangle(1, 4, 63, 63, c_shad1);
         bmp->Rectangle(0, 3, 62, 62, c_shad2);
         break;

      case BOX_SHADOW:
         bmp->Draw3DFrame(0, 0, 62, 62, c_face, c_shad1, c_shad2);
         bmp->Hline(1, 63, 63, BLACK);
         bmp->Vline(63, 1, 63, BLACK);
         break;

      case BUTTON:
         bmp->Draw3DFrame(0,  0, 17, 17, c_deselect, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 18, 17, 35, c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 36, 17, 53, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 54, 17, 71, c_focus, c_shad1, c_shad2);
         break;

      case CHECKBOX:
         bmp->Draw3DFrame(0,  0, 12, 12,  c_sunken, c_shad2, c_shad1);
         bmp->Draw3DFrame(13,  0, 25, 12, c_sunken, c_shad2, c_shad1);
         bmp->DrawXMark(19, 6, 6, c_font);
         bmp->Draw3DFrame(0,  13, 12, 25, c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(13, 13, 25, 25, c_select, c_shad2, c_shad1);
         bmp->DrawXMark(19, 19, 6, c_font);
         bmp->Draw3DFrame(0, 26, 12, 38,  c_face, c_shad2, c_shad1);
         bmp->Draw3DFrame(13, 26, 25, 38, c_face, c_shad2, c_shad1);
         bmp->DrawXMark(19, 32, 6, c_disable);
         bmp->Draw3DFrame(0, 39, 12, 51,  c_focus, c_shad2, c_shad1);
         bmp->Draw3DFrame(13,39, 25, 51,  c_focus, c_shad2, c_shad1);
         bmp->DrawXMark(19, 45, 6, c_font);
         break;

      case CLEAR_BACK:
         bmp->Clear(c_back);
         break;

      case COMBO_BUTTON:
         bmp->Clear(TRANSPARENT);
         bmp->DrawArrow(4,  9, c_font, 3);
         bmp->DrawArrow(4, 29, c_font, 3);
         bmp->DrawArrow(4, 49, c_disable, 3);
         bmp->DrawArrow(4, 69, c_font, 3);
         break;

      case HSLIDER_BACK:
         bmp->Draw3DFrame(0,  9, 17, 12, TRANSPARENT, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 31, 17, 34, TRANSPARENT, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 53, 17, 56, TRANSPARENT, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 75, 17, 78, TRANSPARENT, c_shad2, c_shad1);
         break;

      case HSLIDER_GRIP:
         bmp->Draw3DFrame(0,  0, 10, 21, c_deselect, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 22, 10, 43, c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 44, 10, 65, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 66, 10, 87, c_focus, c_shad1, c_shad2);
         break;

      case ICONEXIT:
         bmp->Draw3DFrame(0,  0, 14, 13, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 14, 14, 27, c_face, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 28, 14, 41, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 42, 14, 55, c_face, c_shad1, c_shad2);
         bmp->DrawXMark(7,  7, 6, c_font);
         bmp->DrawXMark(7, 21, 6, c_font);
         bmp->DrawXMark(7, 35, 6, c_disable);
         bmp->DrawXMark(7, 49, 6, c_font);
         break;

      case ICONMAX:
         bmp->Draw3DFrame(0,  0, 14, 13, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 14, 14, 27, c_face, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 28, 14, 41, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 42, 14, 55, c_face, c_shad1, c_shad2);
         bmp->Rectangle(3,  3, 10, 10, c_font);
         bmp->Rectangle(3, 17, 10, 24, c_font);
         bmp->Rectangle(3, 31, 10, 38, c_disable);
         bmp->Rectangle(3, 45, 10, 52, c_font);
         bmp->Hline(3,  4, 10, c_font);
         bmp->Hline(3, 18, 10, c_font);
         bmp->Hline(3, 32, 10, c_disable);
         bmp->Hline(3, 46, 10, c_font);
         break;

      case ICONMIN:
         bmp->Draw3DFrame(0,  0, 14, 13, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 14, 14, 27, c_face, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 28, 14, 41, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 42, 14, 55, c_face, c_shad1, c_shad2);
         bmp->Rectangle(3,  9, 10, 10, c_font);
         bmp->Rectangle(3, 23, 10, 24, c_font);
         bmp->Rectangle(3, 37, 10, 38, c_disable);
         bmp->Rectangle(3, 51, 10, 52, c_font);
         break;

      case ICONRESTORE:
         bmp->Draw3DFrame(0,  0, 14, 13, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 14, 14, 27, c_face, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 28, 14, 41, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 42, 14, 55, c_face, c_shad1, c_shad2);
         bmp->Rectangle(3,  3, 10, 10, c_font);
         bmp->Rectangle(3, 17, 10, 24, c_font);
         bmp->Rectangle(3, 31, 10, 38, c_disable);
         bmp->Rectangle(3, 45, 10, 52, c_font);
         bmp->Hline(3,  4, 10, c_font);
         bmp->Hline(3, 18, 10, c_font);
         bmp->Hline(3, 32, 10, c_disable);
         bmp->Hline(3, 46, 10, c_font);
         break;

      case LIST:
         bmp->Rectfill(0,  0, 35, 11, c_sunken);
         bmp->Rectfill(0, 12, 35, 23, c_face);
         bmp->Rectfill(0, 24, 35, 35, c_sunken);
         bmp->Rectfill(0, 36, 35, 47, c_face);
         bmp->Rectfill(0, 48, 35, 59, c_select);
         bmp->Rectfill(0, 60, 35, 71, c_select);
         bmp->Rectfill(0, 72, 35, 83, c_face);
         bmp->Rectfill(0, 84, 35, 95, c_face);
         break;

      case MENU_BACK:
         bmp->Draw3DFrame(0, 0, 63, 63, c_face, c_shad1, c_shad2);
         break;

      case MENU_BUTTON:
         bmp->Clear(c_deselect);
         bmp->Rectfill(0, 12, 17, 23, c_select);
         break;

      case MENU_ITEM:
         bmp->Clear(c_face);

         bmp->Rectfill( 2, 2, 45, 13, c_face);

         bmp->Rectfill( 2, 18, 45, 29,   c_select);

         bmp->Hline( 4, 38, 43, c_shad2);
         bmp->Hline( 4, 39, 43, c_shad1);

         bmp->Rectfill( 2, 50, 45, 61, c_face);
         bmp->DrawArrow(40, 53, c_font, 1);

         bmp->Rectfill( 2, 66, 45, 77,   c_select);
         bmp->DrawArrow(40, 69, c_sunken, 1);

         bmp->Rectfill( 2, 82, 45, 93, c_face);
         bmp->DrawCheckMark(5, 85, c_font);

         bmp->Rectfill( 2, 98, 45, 109,   c_select);
         bmp->DrawCheckMark(5, 101, c_sunken);
         break;

      case PANEL_GROOVE:
         bmp->Rectangle(1, 1, 63, 63, c_shad2);
         bmp->Rectangle(0, 0, 62, 62, c_shad1);
         break;

      case PANEL_RAISED:
         bmp->Draw3DFrame(0, 0, 63, 63, c_face, c_shad1, c_shad2);
         break;

      case PANEL_RIDGE:
         bmp->Rectangle(1, 1, 63, 63, c_shad1);
         bmp->Rectangle(0, 0, 62, 62, c_shad2);
         break;

      case PANEL_SUNKEN:
         bmp->Draw3DFrame(0, 0, 63, 63, c_sunken, c_shad2, c_shad1);
         break;

      case PROGRESSH:
         bmp->Clear(TRANSPARENT);
         bmp->Draw3DFrame(0, 0, 11, 17, c_face, c_shad2, c_shad1);
         bmp->Rectfill(2, 20, 10, 33, c_select);
         break;

      case PROGRESSV:
         bmp->Clear(TRANSPARENT);
         bmp->Draw3DFrame(0, 0, 17, 11, c_face, c_shad2, c_shad1);
         bmp->Rectfill(20, 2, 34, 9, c_select);
         break;

      case RADIO:
         bmp->Draw3DCircle(6, 6, 6, c_sunken, c_shad2, c_shad1);
         bmp->Draw3DCircle(19, 6, 6, c_sunken, c_shad2, c_shad1);
         bmp->Circlefill(19, 6, 2, c_font);
         bmp->Draw3DCircle(6, 19, 6, c_select, c_shad2, c_shad1);
         bmp->Draw3DCircle(19, 19, 6, c_select, c_shad2, c_shad1);
         bmp->Circlefill(19, 19, 2, c_font);
         bmp->Draw3DCircle(6, 32, 6, c_face, c_shad2, c_shad1);
         bmp->Draw3DCircle(19, 32, 6, c_face, c_shad2, c_shad1);
         bmp->Circlefill(19, 32, 2, c_disable);
         bmp->Draw3DCircle(6, 45, 6, c_focus, c_shad2, c_shad1);
         bmp->Draw3DCircle(19, 45, 6, c_focus, c_shad2, c_shad1);
         bmp->Circlefill(19, 45, 2, c_font);
         break;

      case SCROLL_DOWN:
         bmp->Clear(TRANSPARENT);
         bmp->Draw3DFrame(0, 0, 15, 15, c_deselect, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 16, 15, 31, c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 32, 15, 47, c_face, c_shad2, c_shad2);
         bmp->Draw3DFrame(0, 48, 15, 63, c_focus, c_shad1, c_shad2);
         bmp->DrawArrow(4, 6, c_font, 3);
         bmp->DrawArrow(5, 23, c_font, 3);
         bmp->DrawArrow(4, 38, c_disable, 3);
         bmp->DrawArrow(4, 54, c_font, 3);
         break;

      case SCROLL_HBACK:
         break;

      case SCROLL_HGRIP:
         bmp->Draw3DFrame(0,   0, 17, 15, c_deselect, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 16, 17, 31,   c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 32, 17, 47,    c_face, c_shad2, c_shad2);
         bmp->Draw3DFrame(0, 48, 17, 63,   c_select, c_shad1, c_shad2);
         break;

      case SCROLL_HGRIPOVERLAY:
         bmp->Clear(TRANSPARENT);
         break;

      case SCROLL_LEFT:
         bmp->Clear(TRANSPARENT);
         bmp->Draw3DFrame(0, 0, 15, 15, c_deselect, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 16, 15, 31, c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 32, 15, 47, c_face, c_shad2, c_shad2);
         bmp->Draw3DFrame(0, 48, 15, 63, c_focus, c_shad1, c_shad2);
         bmp->DrawArrow(5, 4, c_font, 0);
         bmp->DrawArrow(6, 21, c_font, 0);
         bmp->DrawArrow(5, 36, c_disable, 0);
         bmp->DrawArrow(5, 52, c_font, 0);
         break;

      case SCROLL_RIGHT:
         bmp->Clear(TRANSPARENT);
         bmp->Draw3DFrame(0, 0, 15, 15, c_deselect, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 16, 15, 31, c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 32, 15, 47, c_face, c_shad2, c_shad2);
         bmp->Draw3DFrame(0, 48, 15, 63, c_focus, c_shad1, c_shad2);
         bmp->DrawArrow(6, 4, c_font, 1);
         bmp->DrawArrow(7, 21, c_font, 1);
         bmp->DrawArrow(6, 36, c_disable, 1);
         bmp->DrawArrow(6, 52, c_font, 1);
         break;

      case SCROLL_UP:
         bmp->Clear(TRANSPARENT);
         bmp->Draw3DFrame(0, 0, 15, 15, c_deselect, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 16, 15, 31, c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 32, 15, 47, c_face, c_shad2, c_shad2);
         bmp->Draw3DFrame(0, 48, 15, 63, c_focus, c_shad1, c_shad2);
         bmp->DrawArrow(4, 6, c_font, 2);
         bmp->DrawArrow(5, 23, c_font, 2);
         bmp->DrawArrow(4, 38, c_disable, 2);
         bmp->DrawArrow(4, 54, c_font, 2);
         break;

      case SCROLL_VBACK:
         break;

      case SCROLL_VGRIP:
         bmp->Draw3DFrame(0,   0, 15, 11, c_deselect, c_shad1, c_shad2);
         bmp->Draw3DFrame(0, 12, 15, 23,   c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(0, 24, 15, 35,    c_face, c_shad2, c_shad2);
         bmp->Draw3DFrame(0, 36, 15, 47,   c_select, c_shad1, c_shad2);
         break;

      case SCROLL_VGRIPOVERLAY:
         bmp->Clear(TRANSPARENT);
         break;

      case SEPARATORH:
         bmp->Hline(0, 0, 11, c_shad2);
         bmp->Hline(0, 1, 11, c_shad1);
         break;

      case SEPARATORV:
         bmp->Vline(0, 0, 11, c_shad2);
         bmp->Vline(1, 0, 11, c_shad1);
         break;

      case TAB_BUTTON:
         bmp->Clear(TRANSPARENT);
         bmp->Draw3DFrame(0,  0, 17, 19, c_face, c_shad1, c_shad2);
         bmp->Hline(      1, 17, 16,     c_shad1);
         bmp->Putpixel(   0, 17,         TRANSPARENT);
         bmp->Putpixel(  17, 17,         TRANSPARENT);
         bmp->Draw3DFrame(0, 18, 17, 38, c_face, c_shad1, c_shad2);
         bmp->Putpixel(   0, 35,         TRANSPARENT);
         bmp->Putpixel(  17, 35,         TRANSPARENT);
         bmp->Draw3DFrame(0, 36, 17, 55, c_face, c_shad1, c_shad2);
         bmp->Hline(      1, 53, 16,     c_shad1);
         bmp->Putpixel(   0, 53,         TRANSPARENT);
         bmp->Putpixel(  17, 53,         TRANSPARENT);
         bmp->Draw3DFrame(0, 54, 17, 74, c_face, c_shad1, c_shad2);
         bmp->Hline(      1, 71, 16,     c_shad1);
         bmp->Putpixel(   0, 71,         TRANSPARENT);
         bmp->Putpixel(  17, 71,         TRANSPARENT);
         break;

      case TAB_WINDOW:
         bmp->Draw3DFrame(0, 0, 63, 63, c_face, c_shad1, c_shad2);
         break;

      case VSLIDER_BACK:
         bmp->Draw3DFrame( 9, 0, 12, 17, TRANSPARENT, c_shad2, c_shad1);
         bmp->Draw3DFrame(31, 0, 34, 17, TRANSPARENT, c_shad2, c_shad1);
         bmp->Draw3DFrame(53, 0, 56, 17, TRANSPARENT, c_shad2, c_shad1);
         bmp->Draw3DFrame(75, 0, 78, 17, TRANSPARENT, c_shad2, c_shad1);
         break;

      case VSLIDER_GRIP:
         bmp->Draw3DFrame( 0, 0, 21, 10, c_deselect, c_shad1, c_shad2);
         bmp->Draw3DFrame(22, 0, 43, 10, c_select, c_shad2, c_shad1);
         bmp->Draw3DFrame(44, 0, 65, 10, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(66, 0, 87, 10, c_focus, c_shad1, c_shad2);
         break;

      case WINBOTTOM:
         bmp->Draw3DFrame(-2, 0, 31, 7, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(-2, -2, 31, 3, c_face, c_shad1, c_shad2);
         break;

      case WINDOW:
         break;

      case WINGRIP:
         bmp->Rectfill( 0, 0, 29, 17, c_select);
         bmp->Rectfill( 0, 18, 29, 35, c_shad2);
         break;

      case WINLEFT:
         bmp->Draw3DFrame(0, 0, 7, 71, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(4, 0, 10, 71, c_face, c_shad1, c_shad2);
         break;

      case WINRIGHT:
         bmp->Draw3DFrame(0, 0, 7, 71, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(-2, 0, 3, 71, c_face, c_shad1, c_shad2);
         break;

      case WINTEXT:
         bmp->Clear(TRANSPARENT);
         break;

      case WINTOP:
         bmp->Draw3DFrame(-2, 0, 31, 7, c_face, c_shad1, c_shad2);
         bmp->Draw3DFrame(-2, 4, 31, 10, c_face, c_shad1, c_shad2);
         break;
   };
}


////////////////////////////////////////////////////////////////////////////////
// Draws all the default skin bitmaps
void Skin::GenerateDefaultBitmaps() {
   int i;
   for (i=0; i<nBitmaps; ++i) {
      GenerateDefaultBitmap(i);
   }
}


////////////////////////////////////////////////////////////////////////////////
// Sets all the cursors to the default Allegro arrow
void Skin::GenerateDefaultCursors() {
   int i;
   for (i=1; i<nCursors; i++) {
      curList[i]->Create();
   }
}


////////////////////////////////////////////////////////////////////////////////
// Sets all the fonts to the default Allegro font
void Skin::GenerateDefaultFonts() {
   for (int i=0; i<nFonts; i++) {
      fntList[i]->Create();
   }
}


////////////////////////////////////////////////////////////////////////////////
// Sets all the samples to an empty sample
void Skin::GenerateDefaultSamples() {
   for (int i=0; i<nSamples; i++) {
      smpList[i]->Create(8, false, 11025, 0);
   }
}


////////////////////////////////////////////////////////////////////////////////
// Resets the entire skin
void Skin::Reset() {
   Unload();
   ResetColors();
   ResetControls();
   GenerateDefaultFonts();
   GenerateDefaultCursors();
   GenerateDefaultSamples();
   GenerateDefaultBitmaps();
}


////////////////////////////////////////////////////////////////////////////////
// Resets all the object specific info
void Skin::ResetControls() {
   // size of the central area of the bitmaps
   for (int i=0; i<nBitmaps; i++) {
      bmpList[i]->ThickX(-1);
      bmpList[i]->ThickY(-1);
   }

   focus = 0;
   drawDots = 1;
   wallpaperStyle = 3;
   buttonDisplacement = 1;
   buttonAnimationType = 0;
   buttonAnimationLength = 0;
   boxX = 10;
   boxY = 1;
   boxBack = c_face;
   boxAlign = 0;
   winTextPos.x(10);
   winTextPos.y(8);
   winTextAlign = 0;
   winTextBack = 0;
   winExitPos.x(-22);
   winExitPos.y(6);
   winMaxPos.x(-40);
   winMaxPos.y(6);
   winMinPos.x(-56);
   winMinPos.y(6);
   comboAnimationType = 0;
   comboAnimationLength = 0;
   menuHeight = 16;
   menuDelay = 300;
   menuAnimationType = 0;
   menuAnimationLength = 0;
   winAnimationType = 0;
   winAnimationLength = 0;
   tooltipBack = al_map_rgb(255,255,192);
   tooltipBorder = BLACK;
   tooltipAnimationType = 0;
   tooltipAnimationLength = 0;
}


////////////////////////////////////////////////////////////////////////////////
// For loading a bitmap from disk into the bitmap array
Error Skin::LoadBitmap(const char *filename, int i) {
   if (i<0 || i>=nBitmapsEx) {
      return Error(Error::INDEX);
   }

   return bmpList[i]->Load(filename, MASSettings::useVideoMemory ? Bitmap::VIDEO : Bitmap::MEMORY);
}


////////////////////////////////////////////////////////////////////////////////
// For loading a cursor from disk into the cursor array
Error Skin::LoadCursor(const char *filename, int i, int n, int interval) {
   if (i<0 || i>=nCursorsEx) {
      return Error(Error::INDEX);
   }

   Bitmap bmp;
   Error er = bmp.Load(filename);
   if (er != Error::NONE) {
      return er;
   }

   curList[i]->Create(bmp, n);
   curList[i]->SetAnimationInterval(interval);

   return Error(Error::NONE);
}


////////////////////////////////////////////////////////////////////////////////
// Loads a bitmap or truetype font into the font array
Error Skin::LoadFont(const char *filename, int i, int size) {
   if (i<0 || i>=nFontsEx) {
      return Error(Error::INDEX);
   }

   Error e = fntList[i]->Load(filename, size);

   return e;
}


////////////////////////////////////////////////////////////////////////////////
// For loading a sample from disk into the samples array
Error Skin::LoadSample(const char *filename, int i) {
   if (i<0 || i>=nFontsEx) {
      return Error(Error::INDEX);
   }

   return smpList[i]->Load(filename);
}


////////////////////////////////////////////////////////////////////////////////
// A helper function for setting a pointer to a bitmap in the bitmap array or
// for making a copy of it
void Skin::SetBitmap(Bitmap& b, int i, bool copy) {
   if (i>=0 && i<nBitmapsEx) {
      bmpList[i]->Set(b, copy);
   }
}


////////////////////////////////////////////////////////////////////////////////
// A helper function for setting a pointer to a cursor in the cursor array or
// making a copy of it
void Skin::SetCursor(Cursor& c, int i) {
   if (i>=0 && i<nCursorsEx) {
      delete curList[i];
      curList[i] = &c;
   }
}


////////////////////////////////////////////////////////////////////////////////
// A helper function for setting a new font
void Skin::SetFont(Font& f, int i) {
   if (i>=0 && i<nFontsEx) {
      delete fntList[i];
      fntList[i] = &f;
   }
}


////////////////////////////////////////////////////////////////////////////////
// A helper function for setting a pointer to a sample in the samples array or
// making a copy of it
void Skin::SetSample(Sample& s, int i, bool copy) {
   if (i>=0 && i<nSamplesEx) {
      smpList[i]->Set(s, copy);
   }
}


/*
////////////////////////////////////////////////////////////////////////////////
// Colors the skin bitmaps with the specified color and alpha intensity
//TODO: do this using turbofast A5 blending functions
void Skin::Colorize(const Color &col, int alpha) {
   Bitmap *bmp;
   int i, x, y;
   Color pink = TRANSPARENT;

   Reload();
   for (i=0; i<nBitmaps; i++) {
      bmp = bmpList[i];
      for (y=0; y<bmp->h(); y++) {
         for (x=0; x<bmp->w(); x++) {
            Color p = bmp->Getpixel(x, y);
            if (p == pink)
               continue;

            p.Colorize(col, alpha);
            bmp->Putpixel(x, y, p);
         }
      }
   }

   //if (box_back != -1) box_back = MASColorizePixel(box_back, col, alpha);
   c_back.Colorize(col, alpha);
   c_deselect.Colorize(col, alpha);
   c_disable.Colorize(col, alpha);
   c_face.Colorize(col, alpha);
   c_focus.Colorize(col, alpha);
   c_font.Colorize(col, alpha);
   c_select.Colorize(col, alpha);
   c_shad1.Colorize(col, alpha);
   c_shad2.Colorize(col, alpha);
   c_sunken.Colorize(col, alpha);
}
*/

void Skin::PlaySample(int i) {
   Sample s = GetSample(i);
   s.Stop();
   s.Play(MASSettings::soundVolume);
}


Error Skin::GetError() { return lastError; }
Bitmap& Skin::GetBitmap(int i) const { return (i>=0 && i<nBitmapsEx) ? *bmpList[i] : Bitmap::null; }
Cursor& Skin::GetCursor(int i) const { return *curList[i]; }
Font& Skin::GetFont(int i) const { return *fntList[i]; }
Sample& Skin::GetSample(int i) const { return *smpList[i]; }


Skin *theSkin = NULL;

const char *Skin::getBitmapName(int i) {
	return bitmapName[i];
}
