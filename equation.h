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

#ifndef EQUATION

#define EQUATION

#include <math.h>
#include "variables.h"


#define PLUS           1
#define MOINS          2
#define MULT           3
#define DIV            4
#define PUISS          5
#define FACT           6
#define SGNM           7

#define LN             100
#define SH             101
#define CH             102
#define TH             103
#define EXP            104
#define SIN            105
#define COS            106
#define TAN            107
#define ASH            108
#define ACH            109
#define ATH            110
#define INT            111
#define ABS            112
#define RND            113
#define SQRT           114
#define ACOS           115
#define ASIN           116
#define ATAN           117
#define INTG           118
#define FRAC           119
#define CEIL           120

#define LOG            1000
#define NCR            1001
#define MIN            1002
#define MAX            1003
#define MOY            1004
#define RAND           1005
#define SOMME          1006
#define PROD           1007
#define DERIV          1008
#define ITEG           1009
#define PRMV           1010

#define CODE_ERREUR    0xFFFFF0
#define ERR_TROPLONG   0xFFFFF0
#define ERR_SYNTAXE    0xFFFFF1
#define ERR_VAR        0xFFFFF2
#define ERR_MATH       0xFFFFF3
#define ERR_PARAM      0xFFFFF4
#define ERR_RECURSION  0xFFFFF5
#define ERR_SOLUTION   0xFFFFF6

#define TAUX_VARIATION_MAX         50
#define TAUX_VARIATION_DIFF_MAX    70

typedef enum TypeIntegration
{
        RIEMANN, AIRES, GAUSS, PROBA, PRIMITIVE
} TypeIntegration;

typedef enum TypePrimitive
{
        EULER, INTEGRATION
} TypePrimitive;

typedef struct OptionsAlgo
{
        int nombrePointsI, nombrePointsP;
        TypeIntegration calculIntg;
        TypePrimitive calculPrimitive;
} OptionsAlgo;


double Calculer (char *chaineDeCalcul, char variable, double valeur, OptionsAlgo *options);
int CompterCaracteres(char *chaine, char caractere);
int TestNombre(char caractere);
int Factorielle(int nombre);
int RecupererParametres(char chaine[], double param[], int max, char variable, double valeur, char *tab[], int tailleMax);
double CalculSerie(double param[], int taille, int fonction);
int TestMultImp(char chaine[], int position, char variable);
int TestLettre(char caractere);
int TestVariable(char chaine[], int position);
int TestFonction(char chaine[], int position);
char *RechercheParenthese(char *position);
char *RechercheSeparateur(char *position);
int TestSymbole(char caractere);
int TestOperateur(char caractere);
int TestOperateurEx(char caractere);

double Derivee(char equation[], char variable, double valeur);

double Primitive(char equation[], char variable, double valeur, double a, double b, OptionsAlgo options);
double PrimitiveParIntegration(char equation[], char variable, double valeur, double a, double b, OptionsAlgo options);
double PrimitiveEuler(char equation[], char variable, double valeur, double a, double b, int nombrePoints);

double Integration(char equation[], char variable, double valeurDebut, double valeurFin, OptionsAlgo options);
double IntegrationParPrimitive(char equation[], char variable, double valeurDebut, double valeurFin, int nombrePoints);
double IntegrationAires(char equation[], char variable, double valeurDebut, double valeurFin, int nombrePoints);
double IntegrationProba(char equation[], char variable, double valeurDebut, double valeurFin, int nombrePoints);
double IntegrationGauss(char equation[], char variable, double valeurDebut, double valeurFin);
double IntegrationRiemann(char equation[], char variable, double valeurDebut, double valeurFin, int nombrePoints);

double VAbsolue(double x);
double TestContinuite(char equation[], char variable, double a, double b);
double TestContinuitePoint(char equation[], char variable, double a, double b, double *y1, double *y2);
int ConfigEquaParam(char equation[], char variable);
int RecupererEquasParam(char equation[], char equation1[], char equation2[], char variable, int taille);

int ResoudreEquation(char expression[], char inconnue, double debut, double fin);

#include "fonctions.h"

int ExpliciterExpression(char expression[], int taille, char variable, Fonction *tab[], Variable *tab2[], int tailleTab1, int tailleTab2);
int RemplacerExpression(char chaine[], char aRemplacer[], char remplacerPar[], int taille);
int EnleverMultImp(char expression[], char variable);

#endif
