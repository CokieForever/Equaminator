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

#ifndef AFFICHAGE_TEXTE

#define AFFICHAGE_TEXTE


#include "texte_commun.h"

#define TEXTE_FLAGS_BALISES 1
#define TEXTE_FLAGS_TRONQUER 2

typedef struct AFFICHERTEXTE
{
        SDL_Surface **surfaceBlit;
        SDL_Rect *positionBlit;
        char *texte;
        SDL_Color couleurPP,
                  couleurAP;
        int typeBlit,
            espaceH,
            espaceV,
            flags,
            nombrePolices;
        TTF_Font **tableauPolices;
} AFFICHERTEXTE;

int TEXTE_Afficher (SDL_Surface**, SDL_Rect*, char*, int, SDL_Color, SDL_Color, int, int, int, TTF_Font**, int);
int TEXTE_AfficherEx (AFFICHERTEXTE *at);
int TEXTE_ConvertirCouleur(char texte[], SDL_Color *couleur);
int TEXTE_Tronquer(char texte[], int largeurMax, TTF_Font *police);
int TEXTE_ShadedVersBlended(SDL_Surface *surface1, SDL_Rect *positionSurface1, SDL_Surface *surface2, SDL_Rect *positionSurface2);
Uint32 TEXTE_GetPixel(SDL_Surface *surface, int x, int y);
void TEXTE_PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

#endif
