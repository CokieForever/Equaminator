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

#include "boutons.h"


SDL_Event *eventB = NULL;
Menu *menuB = NULL;
SDL_sem *semaphoreB = NULL,
        *semaphoreBAttente = NULL;
SDL_Thread *threadB = NULL;
SDL_Surface *surfaceB = NULL;
int numBoutonB = 0;



SDL_Surface* InitBoutons()
{
    eventB = malloc(sizeof(SDL_Event));
    menuB = malloc(sizeof(Menu));
    semaphoreB = SDL_CreateSemaphore(0);
    semaphoreBAttente = SDL_CreateSemaphore(0);
    threadB = SDL_CreateThread(FonctionThread, NULL);
    surfaceB = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, HAUTEUR, 32, 0, 0, 0, 0);
    SDL_SetColorKey(surfaceB, SDL_SRCCOLORKEY, SDL_MapRGB(surfaceB->format, 0, 0, 0));
    return surfaceB;
}

int RelayerEventBoutons(SDL_Event e, Menu m)
{
    memcpy(eventB, &e, sizeof(SDL_Event));
    memcpy(menuB, &m, sizeof(Menu));

    SDL_SemPost(semaphoreB);
    return 1;
}

int AttendreBoutons()
{
    SDL_SemWait(semaphoreBAttente);
    return numBoutonB;
}

int FonctionThread(void *ptv)
{
    int i, sortie = 0;

    while (1)
    {
        SDL_SemWait(semaphoreB);
        SDL_SemTryWait(semaphoreBAttente);

        SDL_FillRect(surfaceB, 0, SDL_MapRGB(surfaceB->format, 0, 0, 0));
        for (i=0 ; i < menuB->nb && !sortie ; i++)
        {
            menuB->btn[i].dim.x += menuB->pos.x;
            menuB->btn[i].dim.y += menuB->pos.y;
            sortie = TestBouton(*eventB, surfaceB, &(menuB->btn[i]));
            menuB->btn[i].dim.x -= menuB->pos.x;
            menuB->btn[i].dim.y -= menuB->pos.y;
        }

        if (sortie)
           numBoutonB = menuB->btn[i-1].id;
        else numBoutonB = -1;
        sortie = 0;

        SDL_SemPost(semaphoreBAttente);
    }

    return 1;
}

int TestPositionSouris(SDL_Event event, SDL_Rect position, int largeur, int hauteur)
{
    if (largeur <= 0)
       largeur = position.w;
    if (hauteur <= 0)
       hauteur = position.h;

    return event.button.x >= position.x && event.button.x <= position.x + largeur && event.button.y >= position.y && event.button.y <= position.y + hauteur;
}

int TestBouton(SDL_Event e, SDL_Surface* s, Bouton *btn)
{
    int sortie = 0;

    if (!TestPositionSouris(e, btn->dim, 0, 0))
    {
        btn->etat = ZERO;
        return 0;
    }

    if (e.type == SDL_MOUSEBUTTONDOWN)
        btn->etat = BAS;
    else if (e.type == SDL_MOUSEBUTTONUP)
    {
         if (btn->etat == BAS)
            sortie = 1;
         btn->etat = HAUT;
    }
    else if (btn->etat != BAS)
        btn->etat = HAUT;

    if (btn->etat == BAS)
        DessinerRectangle(s, btn->dim, btn->clClic);
    else if (btn->etat == HAUT)
         DessinerRectangle(s, btn->dim, btn->clStd);

    return sortie;
}

int DessinerRectangle(SDL_Surface *s, SDL_Rect dim, SDL_Color cl)
{
    SDL_Surface *rect1 = SDL_CreateRGBSurface(SDL_HWSURFACE, dim.w+2, dim.h+2, 32, 0, 0, 0, 0);
    SDL_FillRect(rect1, 0, SDL_MapRGB(rect1->format, cl.r, cl.g, cl.b));

    int rouge = 0;
    if (cl.r == 0)
        rouge = 255;
    SDL_SetColorKey(rect1, SDL_SRCCOLORKEY, SDL_MapRGB(rect1->format, rouge, 0, 0));

    SDL_Rect pos;
    pos.x = 1;
    pos.y = 1;
    SDL_Surface *rect2 = SDL_CreateRGBSurface(SDL_HWSURFACE, dim.w, dim.h, 32, 0, 0, 0, 0);
    SDL_FillRect(rect2, 0, SDL_MapRGB(rect2->format, rouge, 0, 0));
    SDL_BlitSurface(rect2, NULL, rect1, &pos);

    pos.x = dim.x-1;
    pos.y = dim.y-1;
    SDL_BlitSurface(rect1, NULL, s, &pos);

    SDL_FreeSurface(rect1);
    SDL_FreeSurface(rect2);
    return 1;
}

int DessinerRectangle2(SDL_Surface *s, POINT pt1, POINT pt2, SDL_Color cl)
{
    SDL_Rect rect;

    if (pt1.x < pt2.x)
    {
        rect.x = pt1.x;
        rect.w = pt2.x - pt1.x;
    }
    else
    {
        rect.x = pt2.x;
        rect.w = pt1.x - pt2.x;
    }

    if (pt1.y < pt2.y)
    {
        rect.y = pt1.y;
        rect.h = pt2.y - pt1.y;
    }
    else
    {
        rect.y = pt2.y;
        rect.h = pt1.y - pt2.y;
    }

    return DessinerRectangle(s, rect, cl);
}


