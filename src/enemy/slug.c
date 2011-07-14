/*
Copyright (C) 2009-2011 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../graphics/animation.h"
#include "../entity.h"
#include "../system/properties.h"
#include "../custom_actions.h"
#include "../system/error.h"
#include "../collisions.h"

extern Entity *self;

static void move(void);
static void moveOnCeiling(void);
static void riseToCeiling(void);
static void init(void);
static void slimeWait(void);
static void ceilingSlimeWait(void);
static void slimeTouch(Entity *);
static void creditsMove(void);
static void ceilingSlimeDrip(void);
static void ceilingSlimeDripWait(void);
static void slimePuddleForm(void);

Entity *addSlug(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Slug");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;
	
	e->creditsAction = &init;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->startX = 0;
	
	self->action = strcmpignorecase(self->name, "enemy/upside_down_slug") == 0 ? &riseToCeiling : &move;
	
	self->creditsAction = strcmpignorecase(self->name, "enemy/upside_down_slug") == 0 ? &riseToCeiling : &creditsMove;
}

static void move()
{
	Entity *e;
	
	moveLeftToRight();
	
	self->startX -= self->speed;
	
	if (self->startX <= 0)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Slug Slime");
		}
		
		loadProperties("enemy/slug_slime", e);
		
		setEntityAnimation(e, "GROUND");
		
		e->x = self->face == LEFT ? self->x + self->w - e->w : self->x;
		e->y = self->y + self->h - e->h;
		
		e->action = &slimeWait;
		e->draw = &drawLoopingAnimationToMap;
		e->touch = &slimeTouch;
		
		self->startX = e->w;
	}
}

static void riseToCeiling()
{
	self->flags |= FLY;
	
	self->dirY -= GRAVITY_SPEED * self->weight;
	
	checkToMap(self);
	
	if (self->dirY == 0)
	{
		self->action = &moveOnCeiling;
		
		self->creditsAction = &creditsMove;
	}
}

static void moveOnCeiling()
{
	Entity *e;
	
	if (self->dirX == 0)
	{
		self->face = self->face == RIGHT ? LEFT : RIGHT;
	}

	self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

	if (isAtCeilingEdge(self) == TRUE)
	{
		self->dirX = 0;
	}

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}
	
	self->startX -= self->speed;
	
	if (self->startX <= 0)
	{
		e = getFreeEntity();
		
		if (e == NULL)
		{
			showErrorAndExit("No free slots to add Slug Slime");
		}
		
		loadProperties("enemy/slug_slime", e);
		
		setEntityAnimation(e, "CEILING");
		
		e->x = self->face == LEFT ? self->x + self->w - e->w : self->x;
		e->y = self->y;
		
		e->action = &ceilingSlimeWait;
		e->draw = &drawLoopingAnimationToMap;
		
		e->thinkTime = 180;
		
		self->startX = e->w;
	}
}

static void slimeWait()
{
	self->thinkTime--;
	
	if (self->thinkTime < 60)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}
	
	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
	
	checkToMap(self);
}

static void ceilingSlimeWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "DRIP_FORM");
		
		self->animationCallback = &ceilingSlimeDrip;
	}
	
	checkToMap(self);
}

static void ceilingSlimeDrip()
{
	setEntityAnimation(self, "DRIP");
	
	self->flags &= ~FLY;
	
	self->action = &ceilingSlimeDripWait;
	
	checkToMap(self);
}

static void ceilingSlimeDripWait()
{
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		setEntityAnimation(self, "DRIP_FINISH");
		
		self->animationCallback = &slimePuddleForm;
	}
}

static void slimePuddleForm()
{
	setEntityAnimation(self, "GROUND");
	
	self->touch = &slimeTouch;
	
	self->thinkTime = self->maxThinkTime;
	
	self->action = &slimeWait;
}

static void slimeTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		setCustomAction(other, &stickToFloor, 3, 0, 0);
	}
}

static void creditsMove()
{
	self->face = RIGHT;
	
	setEntityAnimation(self, "STAND");
	
	self->dirX = self->speed;
	
	checkToMap(self);
	
	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
