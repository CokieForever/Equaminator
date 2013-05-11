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

#ifndef DESSIN

#define DESSIN


#include "standards.h"
#include "equation.h"


typedef struct OptionsTrace
{
        int nombrePoints, relierPoints, testDiscontinuite;
        COLORREF couleurTrace, couleurRepere;
        double xmin, xmax, ymin, ymax, xech, yech, rotation, xorigine, yorigine;
        TypeCourbe typeCourbe;
        char variable;
} OptionsTrace;

typedef struct OptionsParam
{
        int bornesAuto, continuerTrace, arreterTrace, pointsConsecutifs, pasFixe;
        int nombrePointsSup, nombrePointsDehors;
        double borne1, borne2, pas;
} OptionsParam;


int TracerCourbe(char equation[], char variable, OptionsTrace ot, OptionsParam op, SDL_Surface *surfaceBlit);
int DessinerRepere (OptionsTrace ot, SDL_Surface *surfaceBlit);
SDL_Rect CalculerOrigine(OptionsTrace ot, int largeurSurface, int hauteurSurface);
void PixelVersCoord(POINT rect, OptionsTrace ot, int w, int h, double *X, double *Y);
POINT CoordVersPixel(double X, double Y, OptionsTrace ot, int w, int h);

#endif
