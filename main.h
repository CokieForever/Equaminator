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

#ifndef MAIN

#define MAIN

#include "equation.h"
#include "dessin.h"

#define LARGEUR          700
#define HAUTEUR          510

#define IDMENUZOOM       2
#define IDMENUAFF        3
#define IDREGLAGES       4
#define IDMENUGRAPH      5
#define IDMENUALG        6
#define IDMENUTAB        7
#define IDZOOMCARRE      8
#define IDZOOMPLUS       9
#define IDZOOMMOINS      10
#define IDZOOMAUTO       11
#define IDZOOMREGLAGES   12
#define IDZOOMDEF        13
#define IDZOOMNORM       14
#define IDGRAPHISCT      15
#define IDGRAPHMAX       16
#define IDGRAPHMIN       17
#define IDGRAPHRESOLY    18
#define IDGRAPHRESOLX    19
#define IDGRAPHITEG      20

#define BLENDED 1
#define SHADED  2
#define SOLID   3


typedef enum EtatBouton
{
    ZERO, BAS, HAUT
} EtatBouton;

typedef struct Bouton
{
    SDL_Rect dim;
    SDL_Color clStd,
              clClic;
    int id;
    EtatBouton etat;
    SDL_Surface *img;
} Bouton;


typedef struct Menu
{
    SDL_Surface *img;
    Bouton *btn;
    SDL_Rect pos;
    int nb;
} Menu;


void CentrerFenetre(HWND hwnd1, HWND hwnd2);
void InitialiserVariables();
int ReglerFenetreOptionsParam(OptionsParam options[]);
int SauverOptionsParam(OptionsParam *options);
int IsItemSelected(HWND listWnd, int item);
int GetFirstSelectedItem(HWND listWnd);
int EntrerFonction(HWND listeWnd, int item, Fonction fonction);
Fonction RecupererFonction(HWND listeWnd, int item);
int MettreAJourTabFonctions(HWND listeWnd, Fonction *tab[], int taille);
int FonctionExiste(HWND listeWnd, char nom[]);
int EntrerVariable(HWND listeWnd, int item, Variable variable);
Variable RecupererVariable(HWND listeWnd, int item);
int MettreAJourTabVariables(HWND listeWnd, Variable *tab[], int taille);
int VariableExiste(HWND listeWnd, char nom[]);
int ChangerMenu(Menu *m);
int TestBoutons(Menu m, SDL_Event event);
void DeclencherEvent();
int DessinerTexte(SDL_Surface *s, SDL_Rect pos, char texte[], TTF_Font *police, SDL_Color clPP, SDL_Color clAp, int type);
void AjouterCoordonnees(SDL_Surface *s, TTF_Font *police);
SDL_Rect PositionnerSecondCurseur(SDL_Surface *s);

Uint32 TransitionDroite(Uint32 interval, void* param);
Uint32 TransitionGauche(Uint32 interval, void* param);
Uint32 Fondu(Uint32 interval, void* param);

LRESULT CALLBACK Options_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OptionsTrace_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Couleur_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OptionsParam_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OptionsAlgo_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OptionsFonctions_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK NouvelleFonction_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OptionsVariables_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK NouvelleVariable_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Rappels_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadFenetreOptions(LPVOID lpParameter);

#include "boutons.h"

#endif
