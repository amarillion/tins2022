#pragma once

#include "component.h"
#include <vector>
#include <allegro5/allegro_font.h>
#include "componentbuilder.h"
#include "data.h"

class IColorGenerator
{
public:
	virtual ALLEGRO_COLOR getColor(int msec, int index) = 0;
	virtual ~IColorGenerator() {}
};

enum class TextStyle { NONE = 0, DROP_SHADOW = 1, CLEAR_BACKGROUND, UNDERLINE };

/*
 * A *span* style,
 * covering things like color, font, bold. and various text effects,
 * but not paragraph effects, like alignment.
 */
class RichStyle {
	std::shared_ptr<RichStyle> parentStyle;

	TextStyle decoration = TextStyle::NONE;
	bool useParentDecoration = true;

	std::shared_ptr<IColorGenerator> letterColorGenerator = nullptr;
	std::shared_ptr<IMotion> letterMotion = nullptr;

	ALLEGRO_COLOR fg = WHITE;
	bool useParentColor = true;

	ALLEGRO_FONT *font = nullptr;
public:

	bool isPerLetter() const { return (getLetterColorGenerator() != nullptr) || (getLetterMotion() != nullptr); }

	TextStyle getDecoration() const {
		return (useParentDecoration && parentStyle) ? parentStyle->decoration : decoration;
	}

	const std::shared_ptr<IColorGenerator> &getLetterColorGenerator() const {
		return (!letterColorGenerator && parentStyle) ? parentStyle->letterColorGenerator : letterColorGenerator;
	}

	const std::shared_ptr<IMotion> getLetterMotion() const {
		return (!letterMotion && parentStyle) ? parentStyle->letterMotion : letterMotion;
	}

	ALLEGRO_COLOR getColor() const {
		return (useParentColor && parentStyle) ? parentStyle->fg : fg;
	}

	const ALLEGRO_FONT *getFont() const {
		return (parentStyle && !font) ? parentStyle->font : font;
	}

	void setParentStyle(const std::shared_ptr<RichStyle> &value) { parentStyle = value; }
	void setDecoration(TextStyle value) { decoration = value; useParentDecoration = false; }
	void setColor(ALLEGRO_COLOR value) { fg = value; useParentColor = false; }
	void setFont(ALLEGRO_FONT *value) { font = value; }
	void setLetterColorGenerator(const std::shared_ptr<IColorGenerator> &value) { letterColorGenerator = value; }
	void setLetterMotion(const std::shared_ptr<IMotion> &value) { letterMotion = value; }

	RichStyle() {}
	RichStyle(TextStyle decoration) : decoration(decoration), useParentDecoration(false) {}
	RichStyle(ALLEGRO_COLOR color) : fg(color), useParentColor(false) {}
	RichStyle(ALLEGRO_FONT *font, ALLEGRO_COLOR color, TextStyle decoration) : decoration(decoration), useParentDecoration(false), fg(color),
			useParentColor(false), font(font) {}
	RichStyle(const std::shared_ptr<IMotion> &letterMotion) :
		letterMotion(letterMotion)
		{}
};


/**
 * a Span: a stretch of text with a single style,
 * or an inline image.
 *
 * Each stretch can have its own style
 * with different colors and fonts. A style is
 * backed by a parent style for values that are
 * not defined.
 *
 * A stretch doesn't have a fixed layout, it may
 * be wrapped if it doesn't fit in a given
 * horizontal space.
 */
class Span
{
private:
	const std::string elem;
	const std::shared_ptr<RichStyle> style;
	const bool fImage;
	ALLEGRO_BITMAP *image;
public:
	Span(const std::string &val, const std::shared_ptr<RichStyle> &style) : elem(val), style(style), fImage(false), image(nullptr) {}
	Span(ALLEGRO_BITMAP *image) : elem(), style(nullptr), fImage(true), image(image) {}
	const std::string &getText() const { return elem; }
	const std::shared_ptr<RichStyle> &getStyle() const { return style; }
	bool isImage() const { return fImage; }
	ALLEGRO_BITMAP *getImage() const { return image; }
};

class RichTextModel : public DataWrapper
{
private:
	std::vector<Span> spans;
	std::shared_ptr<RichStyle> fallbackStyle;
public:
	RichTextModel() : spans() {
		fallbackStyle = std::make_shared<RichStyle>();
	}

	size_t getSize() {
		int result = 0;
		for (auto &i : spans) {
			result += i.getText().size();
		}
		return result;
	}

//	std::string &getText() { return text; }

	size_t getNumSegments() {
		return spans.size();
	}

	const std::string &getSegmentText(int i) {
		return spans[i].getText();
	}

	const Span &getSegment(int i) {
		return spans[i];
	}

	int getSegmentHeight(int i) {
		if (spans[i].isImage()) {
			return al_get_bitmap_height(spans[i].getImage());
		}
		else {
			return al_get_font_line_height(spans[i].getStyle()->getFont());
		}
	}

	int getSegmentWidth(int i) {
		if (spans[i].isImage()) {
			return al_get_bitmap_width(spans[i].getImage());
		}
		else {
			return al_get_text_width(spans[i].getStyle()->getFont(),
				spans[i].getText().c_str());
		}
	}

	const std::shared_ptr<RichStyle> &getSegmentStyle(int i) {
		return spans[i].getStyle();
	}

	const std::shared_ptr<RichStyle> &getFallbackStyle() {
		return fallbackStyle;
	}

	virtual void setText(std::string const &value)
	{
		spans.clear();
		spans.push_back(Span(value, fallbackStyle));
		FireEvent(1);
	}

	void setColor(ALLEGRO_COLOR value) {
		fallbackStyle->setColor(value);
	}

	void setFont(ALLEGRO_FONT *font) {
		assert(font);
		fallbackStyle->setFont(font);
		FireEvent(1);
	}

	void setLetterColorGenerator(const std::shared_ptr<IColorGenerator> &value) {
		fallbackStyle->setLetterColorGenerator(value);
	}

	void setLetterMotion(std::shared_ptr<IMotion> const &value) {
		fallbackStyle->setLetterMotion(value);
	}

	void setDecoration(TextStyle value) {
		fallbackStyle->setDecoration(value);
	}

	void appendText(const std::string &text, const std::shared_ptr<RichStyle> &style) {
		style->setParentStyle(fallbackStyle);
		spans.push_back(Span(text, style));
		FireEvent(1);
	}

	void appendText(const std::string &text) {
		spans.push_back(Span(text, fallbackStyle));
		FireEvent(1);
	}

	void appendImage(ALLEGRO_BITMAP *bmp) {
		spans.push_back(Span(bmp));
		FireEvent(1);
	}

	virtual ~RichTextModel() {}
};

class Text : public Component, public DataListener
{
protected:
	virtual void changed(int code = 0) override = 0;
public:
	enum { OFF, LINE_BY_LINE, LETTER_BY_LETTER };

	virtual void draw(const GraphicsContext &gc) override = 0;
	virtual void update() override = 0;

	virtual void startTypewriter(int mode, int delay = 0) = 0;
	virtual void setAlignment(int value) = 0;
	virtual void setColor (ALLEGRO_COLOR _color) = 0;
	virtual void setLetterColorGenerator (std::shared_ptr<IColorGenerator> generator) = 0;
	virtual void setText (std::string const &value) = 0;
	virtual void setTextModel(std::shared_ptr<RichTextModel> const &value) = 0;
	virtual void setMotion (std::shared_ptr<IMotion> const &_motion) = 0;
	virtual void setLetterMotion (std::shared_ptr<IMotion> const &_letterMotion) = 0;
	virtual void setLetterDelta (int delta) = 0;
	virtual void setDecoration(TextStyle style) = 0;

	virtual void onAnimationComplete(ActionFunc actionFunc) = 0;
	static ComponentBuilder<Text> build(int _align, std::string val);
	static ComponentBuilder<Text> build(ALLEGRO_COLOR _color, std::string val);
	static ComponentBuilder<Text> build(ALLEGRO_COLOR _color, int _align, std::string val);
	static ComponentBuilder<Text> buildf(ALLEGRO_COLOR _color, int _align, const char *msg, ...);
	static ComponentBuilder<Text> buildf(ALLEGRO_COLOR _color, const char *msg, ...);

	virtual void setTextf(const char *msg, ...) = 0;
};
