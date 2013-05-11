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

#ifndef TEXTE_COMMUN

#define TEXTE_COMMUN
#define TEXTE_WIN32

//définition des codes d'erreur
#define TEXTE_ERREUR_PARAMETRE_INVALIDE (-1)
#define TEXTE_ERREUR_SDL                (-2)
#define TEXTE_ERREUR_ALLOC              (-3)
#define TEXTE_ERREUR_THREAD             (-4)
#define TEXTE_ERREUR_OVERFLOW           (-5)

//définition des standards
#define TEXTE_FLAGS_AUCUN   0
#define TEXTE_BLIT_SOLID    1
#define TEXTE_BLIT_SHADED   2
#define TEXTE_BLIT_BLENDED  3
#define SDLK_ENTER          13

#define RED_MASK   0xFF000000
#define GREEN_MASK 0x00FF0000
#define BLUE_MASK  0x0000FF00
#define ALPHA_MASK 0x000000FF

//inclusion des fichiers
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#ifdef TEXTE_WIN32
#include <windows.h>
#endif

//déclaration des fonctions communes
int TEXTE_HauteurMaxi(TTF_Font *police, char texte[]);

#endif
