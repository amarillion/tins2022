#include "graphics.h"
#include <functional>
#include <allegro5/allegro_primitives.h>
#include <vector>

using namespace std;

typedef function<void(const GraphicsContext &gc)> GraphicsStep;

class GraphicsImpl : public Graphics {
private:
	vector<GraphicsStep> steps;
public:	
	virtual void filledRectangle(int x1, int y1, int x2, int y2, const ALLEGRO_COLOR &col) override {
		steps.push_back([=](const GraphicsContext &gc) {
			al_draw_filled_rectangle(x1-gc.xofst, y1-gc.yofst, x2-gc.xofst, y2-gc.yofst, col);
		});
	}

	virtual void draw(const GraphicsContext &gc) {
		for(auto &step: steps) {
			step(gc);
		}
	}
};

shared_ptr<Graphics> Graphics::create() {
	return make_shared<GraphicsImpl>();
}