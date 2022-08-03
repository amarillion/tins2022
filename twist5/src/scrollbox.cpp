#include "scrollbox.h"
#include "util.h"

using namespace std;

int ScrollBox::GetXOffset() const { return viewport->getXofst(); }
int ScrollBox::GetYOffset() const { return viewport->getYofst(); }
void ScrollBox::Center() { /* TODO xofst = ((scroll_w - getw()) / 2); yofst = ((scroll_h - geth()) / 2); */ }
void ScrollBox::SetPos(int xofst, int yofst) { viewport->moveTo(xofst, yofst); }
void ScrollBox::SetScrollSize(int w, int h){ /* TODO scroll_w = w; scroll_h = h; */ }

bool ScrollBox::IsInClientArea(const Point &p) const
{
    return viewport->contains(p.x(), p.y());
}

// set options for scrollers, default = auto
void ScrollBox::SetScrollOption (int newhopt, int newvopt)
{
	if (newhopt != hopt || newvopt != vopt)
	{
		hopt = newhopt;
		vopt = newvopt;
		updateScrollerPositions();
	}
}

void ScrollBox::updateScrollerPositions() {

	//TODO: can also be invisible if SCROLLER_AUTO, and view area is smaller than scroll area...
	bool hVisible = (hopt != SCROLLER_OFF);
	bool vVisible = (vopt != SCROLLER_OFF);

	Bitmap bmpUp =  theSkin->GetBitmap(Skin::SCROLL_UP);
	int buttonw = bmpUp.w();
	int buttonh = bmpUp.h()/4; //TODO -> this is actually a spritesheet... Might be useful to encode that way...

	panel->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, 0, (hVisible ? buttonw : 0), (vVisible ? buttonh : 0));
	viewport->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, bufl, buft, bufr + (hVisible ? buttonw : 0), bufb + (vVisible ? buttonh : 0));

	hScroller->setVisible(hVisible);
	vScroller->setVisible(vVisible);
	hScroller->setLayout(Layout::LEFT_BOTTOM_RIGHT_H, 0, 0, buttonw, buttonh);
	vScroller->setLayout(Layout::RIGHT_TOP_W_BOTTOM, 0, 0, buttonw, buttonh);

}

ScrollBox::ScrollBox(ComponentPtr client) :
	hopt (SCROLLER_AUTO), vopt (SCROLLER_AUTO), hashscroller (false), hasvscroller (false)
{
	viewport = make_shared<ViewPort>();
	panel = make_shared<Panel>();
	hScroller = make_shared<Scroller>();
	vScroller = make_shared<Scroller>();

	Bitmap skinBmp = theSkin->GetBitmap (Skin::PANEL_SUNKEN);

	// Read panel inset from skin
	bufl = (skinBmp.w() - skinBmp.ThickX()) / 2;
	bufr = bufl;
	buft = (skinBmp.h() - skinBmp.ThickY()) / 2;
	bufb = buft;

	viewport->AddListener(this);

	updateScrollerPositions();
	hScroller->SetOrientation(Scroller::HORIZONTAL);
	vScroller->SetOrientation(Scroller::VERTICAL);

	//TODO
//		hScroller->SetIncrement(16);
//		vScroller->SetIncrement(16);

	add(hScroller);
	add(vScroller);
	add(panel);
	add(viewport);

	if (client)
	{
		setClient(client);
	}

	updateGripPositions();
}

Scroller::Scroller()
{
	back = make_shared<ClearScreen>(DARK_GREY);
	add(back);
	bUp = make_shared<Button>();
	add (bUp);
	bDown = make_shared<Button>();
	add (bDown);
	bGrip = make_shared<Button>();
	add (bGrip);

	Bitmap bmpUp = theSkin->GetBitmap(Skin::SCROLL_UP);
	buttonw = bmpUp.w();
	buttonh = bmpUp.h()/4; //TODO -> this is actually a spritesheet... Might be useful to encode that way...
}

void Scroller::SetPosition(int pos) { /* TODO */ }
//void Scroller::SetIncrement(int value) { /* TODO */ }
void Scroller::SetRange (int start, int end) { /* TODO */ }

void Scroller::SetOrientation(int o) {
	orientation = o;
	//   back.orientation = o;
	switch (orientation)
	{
	case Scroller::VERTICAL: {
		bUp->setIcon(Skin::SCROLL_UP);
		bUp->setLayout(Layout::LEFT_TOP_W_H, 0, 0, buttonw, buttonh);

		bDown->setIcon(Skin::SCROLL_DOWN);
		bDown->setLayout(Layout::LEFT_BOTTOM_W_H, 0, 0, buttonw, buttonh);

		bGrip->setBorder(Skin::SCROLL_HGRIP);
		bGrip->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, 0, buttonh, 0, buttonh);

//		back->SetBitmap(Skin::SCROLL_HBACK); //TODO
		//      bGrip->SetOverlay(Skin::SCROLL_HGRIPOVERLAY); //TODO
	} break;
	case Scroller::HORIZONTAL: {
		bUp->setIcon(Skin::SCROLL_LEFT);
		bUp->setLayout(Layout::LEFT_TOP_W_H, 0, 0, buttonw, buttonh);

		bDown->setIcon(Skin::SCROLL_RIGHT);
		bDown->setLayout(Layout::RIGHT_TOP_W_H, 0, 0, buttonw, buttonh);

		bGrip->setBorder(Skin::SCROLL_VGRIP);
		bGrip->setLayout(Layout::LEFT_TOP_RIGHT_BOTTOM, buttonw, 0, buttonw, 0);

//		back->SetBitmap(Skin::SCROLL_VBACK); //TODO
		//      bGrip->SetOverlay(Skin::SCROLL_VGRIPOVERLAY); //TODO
	} break;
	default:
		assert(!"You set invalid value for orientation");
		break;
	}
	UpdateSize();

	// for short scrollbars, grip may become negative in size. TODO  Not sure yet if this is the best solution.
	if (bGrip->getw() < 0 || bGrip->geth() < 0) bGrip->setVisible(false); else bGrip->setVisible(true);
}

void ScrollBox::handleMessage(ComponentPtr src, int msg)
{
	// all messages coming from inside viewport are pushed up
	if (src == viewport)
	{
		pushMsg(msg);
		return;
	}

	int xofst = viewport -> getXofst();
	int yofst = viewport -> getYofst();

	int increment = 0;
	if (msg == MSG_SCROLL_UP)
	{
		increment = 10;
	}
	else if (msg == MSG_SCROLL_DOWN)
	{
		increment = -10;
	}


	if (src == hScroller)
	{
		xofst += increment;
	}
	else if (src == vScroller)
	{
		yofst += increment;
	}

	viewport -> setOfst (xofst, yofst);
}

void Scroller::handleMessage(ComponentPtr src, int msg)
{
	if (msg == MSG_ACTIVATE)
	{
		if (src == bUp)
		{
			pushMsg(ScrollBox::MSG_SCROLL_UP);
		}
		else if (src == bDown)
		{
			pushMsg(ScrollBox::MSG_SCROLL_DOWN);
		}
	}
	return;
}

// tries to make sure the area is visible
void ScrollBox::ScrollToArea (int x, int y, int w, int h)
{
	int nxofst = GetXOffset();
	int nyofst = GetYOffset();

    if (x + w > GetXOffset() + viewport->getw())
    	nxofst = x + w - viewport->getw();
    else if (x < GetXOffset())
        nxofst = x;

    if (y + h > GetYOffset() + viewport->geth())
        nyofst = y + h - viewport->geth();
    else if (y < GetYOffset())
        nyofst = y;

    if (!(nxofst == GetXOffset() && nyofst == GetYOffset()))
    {
    	SetPos (nxofst, nyofst);
    }
}

void ScrollBox::updateGripPositions()
{
	hScroller->setRangeAndPosition(
			-viewport->getXofst(),
			viewport->getw(),
			viewport->getClientWidth());

	vScroller->setRangeAndPosition(
			-viewport->getYofst(),
			viewport->geth(),
			viewport->getClientHeight());
}

void ScrollBox::changed (int code)
{
	updateGripPositions();
}

void Scroller::setRangeAndPosition(int pos, int gripWidth, int max)
{
	int pxMax = 0;
	switch (orientation)
	{
		case Scroller::VERTICAL: pxMax = geth() - buttonh * 2; break;
		case Scroller::HORIZONTAL: pxMax = getw() - buttonw * 2; break;
		default: assert(false); break;
	}

	if (pxMax <= 0)
	{
		bGrip->setVisible(false);
		return;
	}

	double delta =	(double)max / (double)(pxMax);
	int pxGripWidth = bound(0, (int)(gripWidth / delta), pxMax);
	int pxPos = bound(0, (int)(pos / delta), pxMax - pxGripWidth);

	switch (orientation)
	{
		case Scroller::VERTICAL: bGrip->setLayout(Layout::LEFT_TOP_RIGHT_H, 0, buttonh + pxPos, 0, pxGripWidth); break;
		case Scroller::HORIZONTAL: bGrip->setLayout(Layout::LEFT_TOP_W_BOTTOM, buttonw + pxPos, 0, pxGripWidth, 0); break;
	}

	bGrip->setVisible(true);
	UpdateSize();
}
