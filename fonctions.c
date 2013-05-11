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

#include "fonctions.h"


int RAND_init = 1;
Fonction *tabFonctions2[MAX_FONCTIONS] = {NULL};
FonctionDefaut tabFonctionsDefaut2[100] =
{
               {Ln, NULL, "ln", "Logarithme népérien du réel x", "ln(x)", "]0;+I[", LN},
               {sinh, NULL, "sh", "Sinus hyperbolique du réel x", "sh(x)", "R", SH},
               {cosh, NULL, "ch", "Cosinus hyperbolique du réel x", "ch(x)", "R", CH},
               {tanh, NULL, "th", "Tangente hyperbolique du réel x", "th(x)", "R", TH},
               {exp, NULL, "exp", "Exponentielle du réel x", "exp(x)", "R", EXP},
               {sin, NULL, "sin", "Sinus du réel x", "sin(x)", "R", SIN},
               {cos, NULL, "cos", "Cosinus du réel x", "cos(x)", "R", COS},
               {Tan, NULL, "tan", "Tangente du réel x", "tan(x)", "R \\ (Pi/2 + Pi.Z)", TAN},
               {asinh, NULL, "ash", "Argsinus hyperbolique du réel x", "ash(x)", "R", ASH},
               {Ach, NULL, "ach", "Argsinus hyperbolique du réel x", "ach(x)", "[1;+I[", ACH},
               {Ath, NULL, "ath", "Argtangente hyperbolique du réel x", "ath(x)", "]-1;1[", ATH},
               {trunc, NULL, "int", "Troncature entière du réel x", "int(x)", "R", INT},
               {VAbsolue, NULL, "abs", "Valeur absolue du réel x", "abs(x)", "R", ABS},
               {round, NULL, "rnd", "Arrondi à l'unité du réel x", "rnd(x)", "R", RND},
               {Sqrt, NULL, "sqrt", "Racine carrée du réel x", "sqrt(x)", "[0;+I[", SQRT},
               {Asin, NULL, "asin", "Arcsinus du réel x", "asin(x)", "[-1;1]", ASIN},
               {Acos, NULL, "acos", "Arccosinus du réel x", "acos(x)", "[-1;1]", ACOS},
               {atan, NULL, "atan", "Arctangente du réel x", "atan(x)", "R", ATAN},
               {floor, NULL, "intg", "Partie entière du réel x", "intg(x)", "R", INTG},
               {Frac, NULL, "frac", "Partie décimale du réel x", "frac(x)", "R", FRAC},
               {ceil, NULL, "ceil", "Entier directement supérieur au réel x", "ceil(x)", "R", CEIL},

               {NULL, Log, "log", "Logarithme de base a du réel x", "log(x,a)", "]0;+I[ x (]0;+I[ \\ {1})", LOG},
               {NULL, Ncr, "ncr", "Coefficient binomial \"k parmi n\"", "ncr(n,k)", "N x [|0;n|]", NCR},
               {NULL, Min, "min", "Minimum de la série de réels x0...xn (n<100)", "min(x0,...,xn)", "R^n", MIN},
               {NULL, Max, "max", "Maximum de la série de réels x0...xn (n<100)", "max(x0,...,xn)", "R^n", MAX},
               {NULL, Moy, "moy", "Moyenne de la série de réels x0...xn (n<100)", "moy(x0,...,xn)", "R^n", MOY},
               {NULL, Rand, "rand", "Nombre réel aléatoire dans [0;1]", "rand()", "###", RAND},
               {NULL, Somme, "som", "Somme des réels x0 à xn (n<100)", "som(x0,...,xn)", "R^n", SOMME},
               {NULL, Prod, "prod", "Produit des réels x0 à xn (n<100)", "prod(x0,...,xn)", "R^n", PROD},
               {NULL, NULL, "", "Factorielle de l'entier n", "n!", "N", 0},

               {NULL, Drv, "drv", "Nombre dérivé en a de la fonction f, de variable x", "drv(f,x,a)", "###", DERIV},
               {NULL, Iteg, "iteg", "Intégrale en x de a à b de la fonction f, de variable x", "iteg(f,x,a,b)", "###", ITEG},
               {NULL, Prmv, "prmv", "Valeur en a de la primitive de la fonction f, de variable x, qui prend la valeur b en c", "prmv(f,x,a,b,c)", "###", PRMV},

               {NULL, NULL, "", "", "", "", 0}  //gardien
};



double Ln(double x)
{
    if (x<=0)
       return ERR_MATH;
    else return log(x);
}

double Tan(double x)
{
    if (!cos(x))
       return ERR_MATH;
    else return tan(x);
}

double Ach(double x)
{
    if (x<1)
       return ERR_MATH;
    else return acosh(x);
}

double Ath(double x)
{
    if (x<=-1 || x>=1)
       return ERR_MATH;
    else return atanh(x);
}

double Sqrt(double x)
{
    if (x<0)
       return ERR_MATH;
    else return sqrt(x);
}

double Asin(double x)
{
    if (x<-1 || x>1)
       return ERR_MATH;
    else return asin(x);
}

double Acos(double x)
{
    if (x<-1 || x>1)
       return ERR_MATH;
    else return acos(x);
}

double Frac(double x)
{
    return x-trunc(x);
}

double Log(char chaine[], char variable, double valeur, void *donnees)
{
    double param[2] = {0};
    RecupererParametres(chaine, param, 2, variable, valeur, NULL, 0);
    if (param[0] <= 0 || param[1] <= 0 || param[1] == 1)
       return ERR_MATH;
    else if (param[0] >= CODE_ERREUR)
         return ERR_PARAM;
    else if (param[1] >= CODE_ERREUR)
         param[1] = 10;
    return log(param[0])/log(param[1]);
}

double Ncr(char chaine[], char variable, double valeur, void *donnees)
{
    //à améliorer au plus vite !!
    double param[2] = {0};
    RecupererParametres(chaine, param, 2, variable, valeur, NULL, 0);
    if (param[0] >= CODE_ERREUR || param[1] >= CODE_ERREUR)
       return ERR_PARAM;
    else if (floor(param[0]) != param[0] || floor(param[1]) != param[1] || param[1]>param[0] || param[1]<0)
         return ERR_MATH;
    else return Factorielle(param[0])/(Factorielle(param[0]-param[1])*Factorielle(param[1]));
}

double Min(char chaine[], char variable, double valeur, void *donnees)
{
    double param[100] = {0};
    int nombre = RecupererParametres(chaine, param, 100, variable, valeur, NULL, 0);
    return CalculSerie(param, nombre, MIN);
}

double Max(char chaine[], char variable, double valeur, void *donnees)
{
    double param[100] = {0};
    int nombre = RecupererParametres(chaine, param, 100, variable, valeur, NULL, 0);
    return CalculSerie(param, nombre, MAX);
}

double Moy(char chaine[], char variable, double valeur, void *donnees)
{
    double param[100] = {0};
    int nombre = RecupererParametres(chaine, param, 100, variable, valeur, NULL, 0);
    return CalculSerie(param, nombre, MOY);
}

double Rand(char chaine[], char variable, double valeur, void *donnees)
{
    if (!RAND_init)
       srand(time(NULL));
    RAND_init = 1;
    return rand()/(double)RAND_MAX;
}

double Somme(char chaine[], char variable, double valeur, void *donnees)
{
    double param[100] = {0};
    int nombre = RecupererParametres(chaine, param, 100, variable, valeur, NULL, 0);
    return CalculSerie(param, nombre, SOMME);
}

double Prod(char chaine[], char variable, double valeur, void *donnees)
{
    double param[100] = {0};
    int nombre = RecupererParametres(chaine, param, 100, variable, valeur, NULL, 0);
    return CalculSerie(param, nombre, PROD);
}

double Drv(char chaine[], char variable, double valeur, void *donnees)
{
    double param[3] = {0};
    char *tab[3] = {NULL};
    tab[0] = malloc(MAX_CHAINE);
    tab[0][0] = '\0';
    tab[1] = malloc(MAX_CHAINE);
    tab[1][0] = '\0';

    RecupererParametres(chaine, param, 3, variable, valeur, tab, MAX_CHAINE);
    if (param[2] >= CODE_ERREUR || !tab[0][0] || !tab[1][0] || tab[1][1])
    {
       free(tab[0]); free(tab[1]);
       return ERR_PARAM;
    }

    double a = Derivee(tab[0], tab[1][0], param[2]);
    free(tab[0]); free(tab[1]);
    return a;
}

double Iteg(char chaine[], char variable, double valeur, void *donnees)
{
    if (!donnees)
       return ERR_PARAM;
    OptionsAlgo *optn = (OptionsAlgo*) donnees;

    double param[4] = {0};
    char *tab[4] = {NULL};
    tab[0] = malloc(MAX_CHAINE);
    tab[0][0] = '\0';
    tab[1] = malloc(MAX_CHAINE);
    tab[1][0] = '\0';

    RecupererParametres(chaine, param, 4, variable, valeur, tab, MAX_CHAINE);
    if (param[2] >= CODE_ERREUR || !tab[0][0] || !tab[1][0] || tab[1][1] || param[3] >= CODE_ERREUR)
    {
       free(tab[0]); free(tab[1]);
       return ERR_PARAM;
    }

    double a = Integration(tab[0], tab[1][0], param[2], param[3], *optn);
    free(tab[0]); free(tab[1]);
    return a;
}

double Prmv(char chaine[], char variable, double valeur, void *donnees)
{
    if (!donnees)
       return ERR_PARAM;
    OptionsAlgo *optn = (OptionsAlgo*) donnees;

    double param[5] = {0};
    char *tab[5] = {NULL};
    tab[0] = malloc(MAX_CHAINE);
    tab[0][0] = '\0';
    tab[1] = malloc(MAX_CHAINE);
    tab[1][0] = '\0';

    RecupererParametres(chaine, param, 5, variable, valeur, tab, MAX_CHAINE);
    if (param[2] >= CODE_ERREUR || param[3] >= CODE_ERREUR || param[4] >= CODE_ERREUR || !tab[0][0] || !tab[1][0] || tab[1][1])
    {
       free(tab[0]); free(tab[1]);
       return ERR_PARAM;
    }

    double a = Primitive(tab[0], tab[1][0], param[2], param[3], param[4], *optn);
    free(tab[0]); free(tab[1]);
    return a;
}


FonctionDefaut* RecupererTabFnsDef()
{
    return tabFonctionsDefaut2;
}

Fonction** RecupererTabFns()
{
         return tabFonctions2;
}


