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

#ifndef ENTREE_TEXTE

#define ENTREE_TEXTE

#include "texte_commun.h"


//flags
#define TEXTE_FLAGS_SANSTHREAD    3
#define TEXTE_FLAGS_TJSFOCUS      2
#define TEXTE_FLAGS_SLMTNOMBRES   4
#define TEXTE_FLAGS_MOTDEPASSE    8
#define TEXTE_FLAGS_MULTILIGNES   16
#define TEXTE_FLAGS_LECTURESEULE  32
#define TEXTE_FLAGS_DEPLACER      64
#define TEXTE_FLAGS_CENTRER       128
#define TEXTE_FLAGS_ALIGNDROITE   256

//messages
#define TEXTE_MSG_TERMINER        1
#define TEXTE_MSG_MISEAJOUR       2
#define TEXTE_MSG_MODIF           3
#define TEXTE_MSG_ECHAP           4

//constantes de taille
#define TEXTE_MAX_CARACTERES      5000
#define TEXTE_TAILLE_TABLEAU      1000

//autres
#define TEXTE_TEMPS_INFINI        0xffffffff
#ifdef TEXTE_WIN32
#define TEXTE_IDEDIT              216
#endif

//structure principale, utilisée par quasi toutes les fonctions de la librairie
typedef struct EDITIONTEXTE
{
        SDL_Surface *surfaceBlit;
        SDL_Rect positionBlit,
                 positionEdition;
        SDL_Color couleurPP,
                  couleurAP,
                  couleurSelPP,
                  couleurSelAP;
        TTF_Font *police;
        char  *texte,
              texteFixe[200];
        int limiteCaracteres,
            flags,
            typeBlit,
            focus;

        SDL_Surface *surfaceNettoyage,
                    *brush;
        SDL_Rect *positionChar,
                 differenceAuClic;
        SDL_Event event,
                  eventTampon;
        SDL_Thread *thread;
        SDL_sem  *semaphoreE,
                 *semaphoreR,
                 *semaphoreA,
                 *semTermine;
        int allocSurface,
            message,
            messageTampon,
            termine,
            codeRetour,
            premierCaractere,
            dernierCaractere,
            debutSelection,
            debutSelectionInitiale,
            finSelection,
            positionCurseur,
            hauteurMaxi,
            attente,
            deplacement,
            mouvement;

} EDITIONTEXTE;



//fonctions
//utilisables par l'utilisateur
int TEXTE_CreerEdition (SDL_Surface*, SDL_Rect, SDL_Rect, SDL_Color, SDL_Color, SDL_Color, SDL_Color, TTF_Font*, char*, char*, int, int, int, int, EDITIONTEXTE*);
int TEXTE_CreerEditionEx (EDITIONTEXTE *et);
int TEXTE_RelayerEvent (SDL_Event *event, EDITIONTEXTE *et[], int nombreStructures);
int TEXTE_EnvoyerMessage(EDITIONTEXTE *et, int msg, int tempsMax);
int TEXTE_ToutBlitter (EDITIONTEXTE *et[], int nombreStructures, SDL_Surface *ecran);
int TEXTE_ToutEffacer (EDITIONTEXTE *et[], int nombreStructures, int liberer);
int TEXTE_ToutAttendre (EDITIONTEXTE *et[], int nombreStructures);

//destinés au fonctionnement interne de la librairie uniquement
int TEXTE_RecevoirMessage(EDITIONTEXTE *et, int tempsMax);
int TEXTE_FonctionThread (void *ptv);
int TEXTE_Editer (EDITIONTEXTE *et);
int TEXTE_RedessinerEdition (EDITIONTEXTE *et);
int TEXTE_Rendu (EDITIONTEXTE *et, char texte[], SDL_Surface **surfaceTexte, int selectionne);
int TEXTE_TestPositionSouris(SDL_Event event, SDL_Rect position, int largeur, int hauteur);
int TEXTE_CalculerPositionCurseur(EDITIONTEXTE *et);
int TEXTE_InsererCaractere(EDITIONTEXTE *et, char caractere);
int TEXTE_AjusterPremierCaractere(EDITIONTEXTE *et);
int TEXTE_Selectionner(EDITIONTEXTE *et);
int TEXTE_SupprimerSelection(EDITIONTEXTE *et);
char TEXTE_TrierCaractere(EDITIONTEXTE *et, char caractere);
int TEXTE_AjouterBrush (EDITIONTEXTE *et);
SDL_Cursor* TEXTE_CreerCurseur(const char *image[]);
int TEXTE_ModifierCurseurDefaut(SDL_Cursor* curseur);
#ifdef TEXTE_WIN32
LRESULT CALLBACK TEXTE_CallbackFenetre (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI ThreadFenetreCopiage(LPVOID lpParameter);
#endif

#endif
