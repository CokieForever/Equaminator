/*
Equaminator - Small software for mathematical curves drawing
Copyright (C) 2010-2013  Cokie

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef BOUTONS

#define BOUTONS

#include "standards.h"
#include "main.h"


SDL_Surface* InitBoutons();
int RelayerEventBoutons(SDL_Event e, Menu m);
int AttendreBoutons();
int FonctionThread(void *ptv);
int TestPositionSouris(SDL_Event event, SDL_Rect position, int largeur, int hauteur);
int TestBouton(SDL_Event e, SDL_Surface* s, Bouton *btn);
int DessinerRectangle(SDL_Surface *s, SDL_Rect dim, SDL_Color cl);
int DessinerRectangle2(SDL_Surface *s, POINT pt1, POINT pt2, SDL_Color cl);

#endif
