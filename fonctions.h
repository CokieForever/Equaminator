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

#ifndef FONCTIONS

#define FONCTIONS

#include "standards.h"


typedef struct FonctionDefaut
{
        double (*fonction1)(double);
        double (*fonction2)(char*,char,double,void*);
        char nom[10],
             description[MAX_CHAINE],
             appel[100],
             ensemble[30];
        int id;
} FonctionDefaut;


typedef enum TypeCourbe
{
        CARTESIENNE, PARAMETREE, POLAIRE, PARAMPOLAIRE
} TypeCourbe;

typedef struct Fonction
{
        char expression[MAX_CHAINE],
             nom[25],
             variable;
        double borne1, borne2;
        COLORREF couleur;
        TypeCourbe type;
        int active;
} Fonction;


double Ln (double x);
double Tan (double x);
double Ach (double x);
double Ath (double x);
double Sqrt (double x);
double Asin (double x);
double Acos (double x);
double Frac (double x);

double Log (char*, char, double, void*);
double Ncr (char*, char, double, void*);
double Min (char*, char, double, void*);
double Max (char*, char, double, void*);
double Moy (char*, char, double, void*);
double Rand (char*, char, double, void*);
double Somme (char*, char, double, void*);
double Prod (char*, char, double, void*);
double Drv (char*, char, double, void*);
double Iteg (char*, char, double, void*);
double Prmv (char*, char, double, void*);

FonctionDefaut* RecupererTabFnsDef();
Fonction** RecupererTabFns();

#include "equation.h"

#endif
