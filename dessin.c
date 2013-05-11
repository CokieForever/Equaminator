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

#include "dessin.h"


/*char *tableauMappageCourbes[MAX_COURBES];
double *tableauPointsCourbes[MAX_COURBES];
SDL_Rect *tableauCoordsCourbes[MAX_COURBES];*/


int TracerCourbe(char equation2[], char variable, OptionsTrace ot, OptionsParam op, SDL_Surface *surfaceBlit)
{
    if (!equation2 || !surfaceBlit)
       return 0;

    //prise en compte des fonctions définies par l'utilisateur
    char equation[MAX_CHAINE] = "";
    strcpy(equation, equation2);
    int err = ExpliciterExpression(equation, MAX_CHAINE, variable, NULL, NULL, 0, 0);
    if (err >= CODE_ERREUR)
       return err;

    //si courbe paramétrée...
    char tabEquation[2][MAX_CHAINE] = {{0}};
    if (ot.typeCourbe == PARAMETREE || ot.typeCourbe == PARAMPOLAIRE)
    {
       if (!(RecupererEquasParam(equation, tabEquation[0], tabEquation[1], variable, MAX_CHAINE)))
          return ERR_SYNTAXE;
    }


    //création des différentes surfaces et des paramètres
    SDL_Rect position = {0}, prePosition = {0};
    Uint32 couleurTrace = SDL_MapRGB(surfaceBlit->format, GetRValue(ot.couleurTrace), GetGValue(ot.couleurTrace), GetBValue(ot.couleurTrace));

    SDL_Surface *point = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, 1, 32, 0, 0, 0, 0);
    SDL_FillRect(point, NULL, couleurTrace);

    double echelleX = 1.0*surfaceBlit->w / (ot.xmax - ot.xmin),
           echelleY = 1.0*surfaceBlit->h / (ot.ymax - ot.ymin),
           pas = (ot.xmax - ot.xmin) / ot.nombrePoints,
           a = ot.xmin,   //paramètre
           a0 = a,
           y = 0, x = 0,
           test = 1, test2 = 1,
           X,Y,
           max = ot.xmax;

    ot.rotation *= 2*PI/360;
    int debut = 1,
        pointsSup = 0,
        pointsDehors = 0,
        sortie = 0;

    if (op.pasFixe)
       pas = op.pas;
    if (!op.bornesAuto)
    {
       a = op.borne1;
       max = op.borne2;
    }

    //calcul de la position de l'origine dans la fenêtre
    SDL_Rect origine = CalculerOrigine(ot, surfaceBlit->w, surfaceBlit->h);
    DessinerRepere(ot, surfaceBlit);

    /*int numeroCourbe;
    int i=0;
    for (numeroCourbe=0 ; numeroCourbe < MAX_COURBES-1 && tableauCoordsCourbes[numeroCourbe] ; numeroCourbe++);*/


    //tracé du graphe
    do
    {
        switch (ot.typeCourbe)
        {
               case CARTESIENNE:
                    y = Calculer(equation, variable, a, NULL);
                    x = a;
                    break;
               case PARAMPOLAIRE:
               case PARAMETREE:
                    x = Calculer(tabEquation[0], variable, a, NULL);
                    y = Calculer(tabEquation[1], variable, a, NULL);
                    if (ot.typeCourbe == PARAMPOLAIRE)
                    {
                       double x2 = x;
                       x = cos(y)*x2;
                       y = sin(y)*x2;
                    }
                    break;
               case POLAIRE:
                    y = Calculer(equation, variable, a, NULL);
                    x = cos(a)*y;
                    y = sin(a)*y;
                    break;

        }


        //rotation de repere
        Y = -ot.yorigine+cos(ot.rotation)*y-sin(ot.rotation)*x;
        X = -ot.xorigine+cos(ot.rotation)*x+sin(ot.rotation)*y;

        if (y < CODE_ERREUR)
        {
           prePosition.x = position.x;
           prePosition.y = position.y;

           position.x = X*echelleX+origine.x;
           position.y = -Y*echelleY+origine.y;

           if (ot.testDiscontinuite && ot.relierPoints)
           {
              if (ot.typeCourbe == PARAMETREE || ot.typeCourbe == PARAMPOLAIRE)
              {
                 test = TestContinuitePoint(tabEquation[0], variable, a0, a, NULL, NULL);
                 test2 = TestContinuitePoint(tabEquation[1], variable, a0, a, NULL, NULL);
              }
              else test = TestContinuitePoint(equation, variable, a0, a, NULL, NULL);
           }

           if ((position.x >= 0 && position.x < surfaceBlit->w && position.y >= 0 && position.y < surfaceBlit->h)
               || (prePosition.x >= 0 && prePosition.x < surfaceBlit->w && prePosition.y >= 0 && prePosition.y < surfaceBlit->h))
           {
               if (!debut && test && test < CODE_ERREUR && test2 && test2 < CODE_ERREUR && ot.relierPoints)
                  ligne(prePosition.x, prePosition.y, position.x, position.y, couleurTrace, surfaceBlit);
               else SDL_BlitSurface(point, NULL, surfaceBlit, &position);

               /*if (i<=0 || (tableauCoordsCourbes[numeroCourbe][i].x != tableauCoordsCourbes[numeroCourbe][i-1].x
                            && tableauCoordsCourbes[numeroCourbe][i].y != tableauCoordsCourbes[numeroCourbe][i-1].y))
                  tableauCoordsCourbes[numeroCourbe][i] = position;

               if (i>0 && tableauCoordsCourbes[numeroCourbe][i].x != tableauCoordsCourbes[numeroCourbe][i-1].x
                       && tableauCoordsCourbes[numeroCourbe][i].y != tableauCoordsCourbes[numeroCourbe][i-1].y)
               {*/

               if (op.pointsConsecutifs)
                  pointsDehors = 0;
           }
           else pointsDehors++;
        }

        a0 = a;
        if (a < floor(a+pas) && a+pas > ceil(a))   //favorisation des nombres entiers
           a = ceil(a);
        else a += pas;

        debut = 0;

        if (a > max && op.continuerTrace)
           pointsSup++;

        if ((a > max && !op.continuerTrace)
           || (op.continuerTrace && a > max && (pointsSup > op.nombrePointsSup || pointsDehors > 0))
           || (op.arreterTrace && pointsDehors > op.nombrePointsDehors))
        sortie = 1;


    } while (!sortie);


    SDL_FreeSurface(point);
    return 1;
}

int DessinerRepere (OptionsTrace ot, SDL_Surface *surfaceBlit)
{
    if (!surfaceBlit)
       return 0;

    //création des différentes surfaces
    SDL_Rect position = {0};
    position.w = surfaceBlit->w;
    position.h = 1;
    position.y = 2;

    SDL_Surface *axeX = SDL_CreateRGBSurface(SDL_HWSURFACE, surfaceBlit->w, 3, 32, 0, 0, 0, 0);
    Uint32 couleurRepere = SDL_MapRGB(surfaceBlit->format, GetRValue(ot.couleurRepere), GetGValue(ot.couleurRepere), GetBValue(ot.couleurRepere)),
           couleurTransparente = SDL_MapRGB(surfaceBlit->format, 0, 0, 1);
    SDL_FillRect(axeX, NULL, couleurTransparente);
    SDL_FillRect(axeX, &position, couleurRepere);
    SDL_SetColorKey(axeX, SDL_SRCCOLORKEY, couleurTransparente);

    SDL_Surface *axeY = SDL_CreateRGBSurface(SDL_HWSURFACE, 3, surfaceBlit->h, 32, 0, 0, 0, 0);
    position.w = 1;
    position.h = surfaceBlit->h;
    position.y = 0;
    position.x = 0;
    SDL_FillRect(axeY, NULL, couleurTransparente);
    SDL_FillRect(axeY, &position, couleurRepere);
    SDL_SetColorKey(axeY, SDL_SRCCOLORKEY, couleurTransparente);

    SDL_Surface *graduation = SDL_CreateRGBSurface(SDL_HWSURFACE, 2, 2, 32, 0, 0, 0, 0);
    SDL_FillRect(graduation, NULL, couleurRepere);

    //calcul de la position de l'origine dans la fenêtre
    SDL_Rect origine = CalculerOrigine(ot, surfaceBlit->w, surfaceBlit->h);
    double echelleX = 1.0*surfaceBlit->w / (ot.xmax - ot.xmin),
           echelleY = 1.0*surfaceBlit->h / (ot.ymax - ot.ymin);

    //dessin des graduations sur les axes
    position.y = 0;
    double i;
    for (i = ot.xech ; position.x <= surfaceBlit->w ; i += ot.xech)
    {
        position.x = i*echelleX+origine.x;
        SDL_BlitSurface(graduation, NULL, axeX, &position);
    }
    for (i = -ot.xech ; i*echelleX+origine.x >= 0 ; i -= ot.xech)
    {
        position.x = i*echelleX+origine.x;
        SDL_BlitSurface(graduation, NULL, axeX, &position);
    }

    position.x = 1;
    for (i = ot.yech ; position.y <= surfaceBlit->h ; i += ot.yech)
    {
        position.y = i*echelleY+origine.y;
        SDL_BlitSurface(graduation, NULL, axeY, &position);
    }
    for (i = -ot.yech ; i*echelleY+origine.y >= 0; i -= ot.yech)
    {
        position.y = i*echelleY+origine.y;
        SDL_BlitSurface(graduation, NULL, axeY, &position);
    }


    //ajout des axes sur le graphe
    position.x = 0;
    position.y = origine.y-2;
    SDL_BlitSurface(axeX, NULL, surfaceBlit, &position);
    position.x = origine.x;
    position.y = 0;
    SDL_BlitSurface(axeY, NULL, surfaceBlit, &position);

    SDL_FreeSurface(axeX);
    SDL_FreeSurface(axeY);
    return 1;
}

SDL_Rect CalculerOrigine(OptionsTrace ot, int largeurSurface, int hauteurSurface)
{
    double echelleX = 1.0*largeurSurface / (ot.xmax - ot.xmin),
           echelleY = 1.0*hauteurSurface / (ot.ymax - ot.ymin);

    SDL_Rect origine;
    origine.x = -ot.xmin*echelleX;
    origine.y = ot.ymax*echelleY;

    return origine;
}

void PixelVersCoord(POINT rect, OptionsTrace ot, int w, int h, double* X, double* Y)
{
    double echelleX = 1.0*(ot.xmax - ot.xmin) / w,
           echelleY = 1.0*(ot.ymax - ot.ymin) / h;

    SDL_Rect origine = CalculerOrigine(ot, w, h);
    double x = (rect.x-origine.x)*echelleX,
           y = (origine.y-rect.y)*echelleY;

    ot.rotation *= 2*PI/360;

    *Y = cos(ot.rotation)*y+sin(ot.rotation)*x+ot.yorigine*cos(ot.rotation)+ot.xorigine*sin(ot.rotation);
    *X = cos(ot.rotation)*x-sin(ot.rotation)*y-ot.yorigine*sin(ot.rotation)+ot.xorigine*cos(ot.rotation);

    return;
}

POINT CoordVersPixel(double x, double y, OptionsTrace ot, int w, int h)
{
    double echelleX = 1.0*w / (ot.xmax - ot.xmin),
           echelleY = 1.0*h / (ot.ymax - ot.ymin);

    double Y = -ot.yorigine+cos(ot.rotation)*y-sin(ot.rotation)*x,
           X = -ot.xorigine+cos(ot.rotation)*x+sin(ot.rotation)*y;

    SDL_Rect origine = CalculerOrigine(ot, w, h);
    POINT point = {X*echelleX+origine.x, -Y*echelleY+origine.y};

    return point;
}
