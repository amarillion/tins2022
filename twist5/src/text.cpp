#include "text.h"
#include <string>
#include <sstream>
#include "color.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "mainloop.h"
#include "textstyle.h"

using namespace std;
using namespace std::placeholders;

struct SpanLayout {
private:
	string elem;
	shared_ptr<RichStyle> style = nullptr;
	int xco = 0;
	int yco = 0;
	bool fImage = false;
	ALLEGRO_BITMAP *image = nullptr;
public:
	const string &getText() const { return elem; }
	const shared_ptr<RichStyle> &getStyle() const { return style; }
	SpanLayout(const string &elem, const shared_ptr<RichStyle> &style, int xco, int yco) :
		elem(elem), style(style), xco(xco), yco(yco), fImage(false) {}
	SpanLayout(ALLEGRO_BITMAP *image, int xco, int yco) :
		xco(xco), yco(yco), fImage(true), image(image) {}

	/** number of 'typewriter' steps.
	 * 1 for images
	 * length of string otherwise.
	 */
	size_t getSteps() {
		return fImage ? 1 : elem.size();
	}

	int getx() const { return xco; }
	int gety() const { return yco; }
	bool isImage() { return fImage; }
	ALLEGRO_BITMAP *getImage() { return image; }
};

class TextImpl : public Text
{
private:
	int align;

	int letterDelta;

	std::shared_ptr<RichTextModel> model;

	unsigned int typewriterPos;
	int typewritingMode;
	int typewriterDelay;

	std::vector<SpanLayout> elems;

	size_t showlines = 0;
	int lineh;
	bool blink;

	void split (const std::string &s);

	void refresh() {
		showlines = elems.size();
		blink = false;
	}

	ActionFunc actionFunc;
protected:
	virtual void changed(int code = 0) override
	{
		elems.clear();
		int xco = 0;
		int yco = 0;
		int maxy = 0;
		int lasth = 0;

		int ascent = 0; // font ascent, if defined.

		for (size_t i = 0; i < model->getNumSegments(); ++i) {

			const Span &span = model->getSegment(i);
			if (span.isImage()) {
				// calculate a delta to make the image sit on the same base line as text
				int delta = (ascent == 0) ? 0 : ascent - al_get_bitmap_height(span.getImage());
				elems.push_back(SpanLayout(span.getImage(), xco, yco + delta));
				xco += model->getSegmentWidth(i);
				lasth = model->getSegmentHeight(i);
				continue;
			}
			else {
				stringstream ss(model->getSegmentText(i));
				auto style = model->getSegmentStyle(i);
				if (style->getFont() == nullptr) {
					// TODO: unfortunately this is null too often to turn this into an assertion... 
					cerr << "WARN: Missing font" << endl;
					return; 
				}

				// remember ascent for next iteration
				ascent = al_get_font_ascent(style->getFont());
				lasth = model->getSegmentHeight(i);

				string item;

				bool nextLine = false;

				while(std::getline(ss, item, '\n')) {
					if (nextLine) { yco += model->getSegmentHeight(i); xco = 0; }
					elems.push_back(SpanLayout(item, style, xco, yco));
					xco += al_get_text_width(style->getFont(), item.c_str());
					nextLine = true;
				}
			}
		}

		// automatically adjust height to layout.
		// NOTE: if we do vertical alignment, height can't be adjusted automatically here:
		h = yco + lasth;

		refresh();
	}

public:

	enum { OFF, LINE_BY_LINE, LETTER_BY_LETTER };

	TextImpl(ALLEGRO_COLOR color, int _align, string const &aText) :
		align(_align),
		letterDelta(100), model(),
		typewriterPos(0), typewritingMode(OFF), typewriterDelay(0),
		elems(), lineh(0), blink(false), actionFunc()
	{
		model = make_shared<RichTextModel>(); // generate default model
		setColor(color);
		model->setColor(color);
		model->setText(aText);

		model->AddListener(this);
		changed(1); // update from model
		setDimension(300, 20); //TODO: better default w, h...
	}

	virtual void draw(const GraphicsContext &gc) override;
	virtual void update() override;

	virtual void setFont(ALLEGRO_FONT *value) override {
		model->setFont(value);
	}

	void startTypewriter(int mode, int delay = 0)
	{
		typewritingMode = mode;
		switch (mode)
		{
		case LETTER_BY_LETTER:
			typewriterPos = 0;
			typewriterDelay = (delay <= 0) ? 1 : delay;
			break;
		case LINE_BY_LINE:
			showlines = 1;
			typewriterDelay = (delay <= 0) ? 48 : delay;
			break;
		default:
			break;
		}
	}

	virtual void setAlignment(int value) override
	{ align = value; changed(1); }

	virtual void setColor (ALLEGRO_COLOR _color) override
	{ model->setColor(_color); }

	virtual void setLetterColorGenerator (std::shared_ptr<IColorGenerator> generator) override
	{ model->setLetterColorGenerator(generator); }

	virtual void setText (std::string const &value) override
	{ model->setText(value); }

	virtual void setTextModel(std::shared_ptr<RichTextModel> const &value) override
	{
		if (model != value) {
			model = value;
			model->AddListener(this);
			changed(1);
		}
	}

	virtual void setMotion (std::shared_ptr<IMotion> const &_motion) override
	{ motion = _motion; }

	virtual void setLetterMotion (std::shared_ptr<IMotion> const &_letterMotion) override
	{ model->setLetterMotion(_letterMotion); }

	virtual void setDecoration (TextStyle value) override
	{ model->setDecoration(value); }

	virtual void setLetterDelta (int delta) override { letterDelta = delta; }

	virtual void onAnimationComplete(ActionFunc aActionFunc) override {
		actionFunc = aActionFunc;
	}

	void handleAnimationComplete() {
		pushMsg (MSG_ANIMATION_DONE);
		if (actionFunc) { actionFunc(); }
	}

	virtual void setTextf(const char *msg, ...) override
	{
		char buf[256];

		va_list ap;
		va_start(ap, msg);
		vsnprintf (buf, sizeof(buf), msg, ap);
		va_end(ap);

		model->setText(buf);
	}

	void drawTextSpan(const SpanLayout &i, int xco, int ybase, bool typewriterActive, int typewriterRemain) {

		auto style = i.getStyle();
		function<void(const ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, int flags, char const *text)> textFunc;

		switch (style->getDecoration())
		{
		case TextStyle::DROP_SHADOW: textFunc = bind (draw_shaded_text, _1, _2, BLACK, _3, _4, _5, _6); break;
		case TextStyle::CLEAR_BACKGROUND: textFunc = bind (draw_text_with_background, _1, _2, BLACK, _3, _4, _5, _6); break;
		case TextStyle::UNDERLINE: textFunc = draw_text_with_underline; break;
		case TextStyle::NONE: default: textFunc = al_draw_text; break;
		}

		const string &strVal = i.getText();

		string activeText = typewriterActive ? strVal.substr(0, typewriterRemain) : strVal;

		if (!style->isPerLetter()) {
			float xx = xco + i.getx();
			float yy = ybase + i.gety();
			textFunc(style->getFont(), style->getColor(), xx, yy, ALLEGRO_ALIGN_LEFT, activeText.c_str());
		}
		else {
			int xx = xco;
			int pos;
			char textbuf[2] = " ";
			int tlen = activeText.size();
			int ddx = 0;
			int ddy = 0;

			for (int pos = 0; pos < tlen; ++pos)
			{
				ALLEGRO_COLOR lettercolor =
					(style->getLetterColorGenerator() == nullptr) ?
					style->getColor() :
					style->getLetterColorGenerator()->getColor(MSEC_FROM_TICKS(counter), pos);
				textbuf[0] = activeText[pos];

				if (style->getLetterMotion())
				{
					ddx = style->getLetterMotion()->getdx(counter + letterDelta * pos);
					ddy = style->getLetterMotion()->getdy(counter + letterDelta * pos);
				}

				textFunc(style->getFont(), lettercolor, xx + ddx + i.getx(), ybase + ddy + i.gety(), ALLEGRO_ALIGN_LEFT, textbuf);

				xx += al_get_text_width(style->getFont(), textbuf);
			}
		}

	}
};

void TextImpl::draw(const GraphicsContext &gc)
{
	int ybase = gety() + gc.yofst;
	int xbase = getx() + gc.xofst;

	if (motion)
	{
		xbase += motion->getdx(counter);
		ybase += motion->getdy(counter);
	}

	int lineRemain = (typewritingMode == LINE_BY_LINE) ? showlines : elems.size();
	size_t typewriterRemain = typewriterPos;

	string partial;
	for (vector<SpanLayout>::iterator i = elems.begin(); i != elems.end() && lineRemain > 0; ++i, lineRemain--)
	{
		bool typewriterActive = (typewritingMode == LETTER_BY_LETTER) && i->getSteps() >= typewriterRemain;
		if (typewriterActive && typewriterRemain == 0) break;

		if (i->isImage()) {
			al_draw_bitmap(i->getImage(), xbase + i->getx(), ybase + i->gety(), 0);
			typewriterRemain--;
		}
		else {
			auto style = i->getStyle();
			if (!style->getFont()) continue; //TODO: warn("Attempt to draw text without a valid font")

			const string &strVal = i->getText();
			string activeText = typewriterActive ? strVal.substr(0, typewriterRemain) : strVal;

			//TODO: aligning right and center won't work properly if there is more than one
			//span on the same line
			int xco;
			switch (align)
			{
			case ALLEGRO_ALIGN_RIGHT:
				xco = xbase + getw() - al_get_text_width(style->getFont(), activeText.c_str()); break;
			case ALLEGRO_ALIGN_CENTER:
				xco = xbase + (getw() - al_get_text_width(style->getFont(), activeText.c_str())) / 2; break;
			default:
				xco = xbase; break;
			}

			drawTextSpan(*i, xco, ybase, typewriterActive, typewriterRemain);
			typewriterRemain -= activeText.size();
		}
	}

}

void TextImpl::update() {
	Component::update();

	switch (typewritingMode)
	{
	case Text::LINE_BY_LINE:
	{
		if (counter % typewriterDelay == 0)
		{
			showlines++;
			if (showlines > elems.size())
			{
				handleAnimationComplete();
				typewritingMode = Text::OFF;
			}
		}
		break;
	}
	case Text::LETTER_BY_LETTER:
	{
		if (counter % typewriterDelay == 0)
		{
			typewriterPos++;

			// send message when animation is finished
			if (typewriterPos >= model->getSize())
			{
				handleAnimationComplete();
				typewritingMode = Text::OFF;
				typewriterPos = 0;
			}
		}
		break;
	}
	default:
		// do nothing;
		break;
	}
}

ComponentBuilder<Text> Text::build(int _align, std::string val)
{
	//TODO: look up colour from skin.
	return build(BLACK, _align, val);
}

ComponentBuilder<Text> Text::build(ALLEGRO_COLOR _color, std::string val)
{
	return build(_color, ALLEGRO_ALIGN_CENTER, val);
}

ComponentBuilder<Text> Text::build(ALLEGRO_COLOR _color, int _align, std::string val)
{
	auto result = make_shared<TextImpl>(_color, _align, val);
	return ComponentBuilder<Text>(result);
}

ComponentBuilder<Text> Text::buildf(ALLEGRO_COLOR _color, int _align, const char *msg, ...)
{
	char buf[256];

	va_list ap;
	va_start(ap, msg);
	vsnprintf (buf, sizeof(buf), msg, ap);
	va_end(ap);

	auto result = make_shared<TextImpl>(_color, _align, buf);
	return ComponentBuilder<Text>(result);
}

ComponentBuilder<Text> Text::buildf(ALLEGRO_COLOR _color, const char *msg, ...)
{
	char buf[256];

	va_list ap;
	va_start(ap, msg);
	vsnprintf (buf, sizeof(buf), msg, ap);
	va_end(ap);

	auto result = make_shared<TextImpl>(_color, ALLEGRO_ALIGN_LEFT, buf);
	return ComponentBuilder<Text>(result);
}
