#ifndef VIEW_H
#define VIEW_H

#include "tegel5.h"
#include "player.h"
#include "component.h"

class Game;

class View : public Component
{
	private:
		int camera_x, camera_y;

	public:
		View(Game *parent) : parent(parent) {}
		void init (int numPlayers, int player, int pw, int ph);
		
		int status_x, status_y;
		Player *player;
		Game *parent;

		virtual void draw(const GraphicsContext &gc) override;
		virtual void update() override;
};

#endif
