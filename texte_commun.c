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

#include "texte_commun.h"

//fonctions communes
int TEXTE_HauteurMaxi(TTF_Font *police, char texte[])
{
    /*
    renvoie la hauteur maximum que peut avoir un texte avec une police donn�e
    si le texte est NULL, ce sera la hauteur maximum d'un caract�re de la police qui sera renvoy�e
    */

    SDL_Color couleur = {0};
    SDL_Surface *surfaceTexte = NULL;

    //verification des param�tres pass�s
    if (!police)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    if (!texte)
       return TTF_FontHeight(police);

    /*
    Ceci est une alternative � TTF_FontHeight

    char texte2[256] = {0};
    {
       int i;
       for (i = 1 ; i < 256 ; i++)
           texte2[i-1] = i;
       texte = texte2;
    }
    */

    surfaceTexte = TTF_RenderText_Shaded(police, texte, couleur, couleur);   //cr�ation d'une surface pour mesurer la hauteur
    if (!surfaceTexte)
       return TEXTE_ERREUR_SDL;

    int hauteur = surfaceTexte->h;
    SDL_FreeSurface(surfaceTexte);

    return hauteur;
}
