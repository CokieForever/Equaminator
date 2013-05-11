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

#include "entree_texte.h"

//variables globales
int TEXTE_inserer = 1;   //d�termine s'il faut ins�rer ou remplacer les caract�res. Peut �tre modifi� avec la touche INS.
char TEXTE_copiage[TEXTE_MAX_CARACTERES] = {0};   //buffer pour le copier-coller
SDL_sem *TEXTE_semTTF = NULL;   //s�maphore de protection de SDL_ttf : elle �vite deux appels simultan�s de la librairie, ce qui cause un comportement ind�termin�
EDITIONTEXTE *TEXTE_tableau[TEXTE_TAILLE_TABLEAU] = {NULL};    //tableau rassemblant tous les pointeurs vers les EDITIONTEXTE cr��s
SDL_sem *TEXTE_semaphoreTableau = NULL;  //s�maphore de protection du tableau des pointeurs, pour emp�cher deux acc�s simultan�s
SDL_Cursor *TEXTE_curseurEdition = NULL,  //curseurs d'affichage
           *TEXTE_curseurDeplacement = NULL,
           *TEXTE_curseurNormal = NULL;
SDL_Surface *TEXTE_brush = NULL;
int TEXTE_curseurActuel = 0;
#ifdef TEXTE_WIN32
HWND TEXTE_fenetre = NULL;
HANDLE TEXTE_thread = NULL;
#endif

const char *TEXTE_dataCurseurEdition[] = {   //donn�es XPM pour construire les curseurs (par The Gimp)
      "16 16 3 1",
      " 	c None",
      ".	c #FFFFFF",
      "+	c #000000",
      "                ",
      "     .. ..      ",
      "     .+++.      ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "     .+++.      ",
      "     .. ..      ",
      "                ",
      "7,7"
};

const char *TEXTE_dataCurseurDeplacement[] = {
      "16 16 3 1",
      " 	c None",
      ".	c #000000",
      "+	c #FFFFFF",
      "                ",
      "                ",
      "        .       ",
      "       .+.      ",
      "      .+++.     ",
      "      ..+..     ",
      "    .. .+. ..   ",
      "   .+...+...+.  ",
      "  .+++++++++++. ",
      "   .+...+...+.  ",
      "    .. .+. ..   ",
      "      ..+..     ",
      "      .+++.     ",
      "       .+.      ",
      "        .       ",
      "                ",
      "8,8"
};



int TEXTE_CreerEdition (SDL_Surface *surfaceBlit,
                        SDL_Rect positionBlit,
                        SDL_Rect positionEdition,
                        SDL_Color couleurPP,
                        SDL_Color couleurAP,
                        SDL_Color couleurSelPP,
                        SDL_Color couleurSelAP,
                        TTF_Font *police,
                        char texte[],
                        char texteFixe[],
                        int limiteCaracteres,
                        int typeBlit,
                        int focus,
                        int flags,
                        EDITIONTEXTE* et)
{
    //une variante de TEXTE_CreerEditionEx() qui permet de ne pas avoir � remplir la structure soi-m�me
    if (!et)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    memset(et, 0, sizeof(EDITIONTEXTE));

    et->surfaceBlit = surfaceBlit;
    et->positionBlit = positionBlit;
    et->positionEdition = positionEdition;
    et->couleurPP = couleurPP;
    et->couleurAP = couleurAP;
    et->couleurSelPP = couleurSelPP;
    et->couleurSelAP = couleurSelAP;
    et->police = police;
    et->texte = texte;
    if (texteFixe)
    {
       strncpy(et->texteFixe, texteFixe, 200);
       et->texteFixe[199] = '\0';
    }
    else et->texteFixe[0] = '\0';
    et->limiteCaracteres = limiteCaracteres;
    et->typeBlit = typeBlit;
    et->focus = focus;
    et->flags = flags;

    return TEXTE_CreerEditionEx(et);
}

int TEXTE_CreerEditionEx (EDITIONTEXTE *et)
{
    /*
    Fonction de cr�ation d'�dition de texte.
    Pour un mode d'emploi, voir le fichier texte correspondant.
    */

    //v�rification des param�tres en entr�e
    if (!et || (!et->surfaceBlit && et->positionBlit.w <= 0) || !et->police)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    //cr�ation de la s�maphore de protection TTF (voir la d�claration)
    if (!TEXTE_semTTF)
           TEXTE_semTTF = SDL_CreateSemaphore(1);

    //ajustement des param�tres
    SDL_SemWait(TEXTE_semTTF);   //voir la d�claration de TEXTE_semTTF
    et->hauteurMaxi = TEXTE_HauteurMaxi(et->police, NULL);
    SDL_SemPost(TEXTE_semTTF);

    if (et->positionBlit.h <= 0)
       et->positionBlit.h = et->hauteurMaxi;

    if (!et->surfaceBlit)
    {
       et->surfaceBlit = SDL_CreateRGBSurface(SDL_HWSURFACE, et->positionBlit.w + et->positionBlit.x, et->positionBlit.h + et->positionBlit.y, 32, 0, 0, 0, 0);
       et->allocSurface = 1;
       SDL_FillRect(et->surfaceBlit, 0, SDL_MapRGB(et->surfaceBlit->format, et->couleurAP.r, et->couleurAP.g, et->couleurAP.b));
    }
    else et->allocSurface = 0;
    if (!et->surfaceBlit)
       return TEXTE_ERREUR_SDL;
    if (et->positionBlit.w <= 0)
       et->positionBlit.w = et->surfaceBlit->w - et->positionBlit.x;
    if (et->limiteCaracteres <= 0)
       et->limiteCaracteres = TEXTE_MAX_CARACTERES;
    if (!et->texte)
       et->texte = malloc(et->limiteCaracteres);
    if (!et->texte)
       return TEXTE_ERREUR_ALLOC;
    memset(et->texte, 0, et->limiteCaracteres);
    et->attente = 0;
    et->message = 0;
    et->debutSelection = -1;
    et->finSelection = -1;
    et->premierCaractere = 0;
    et->deplacement = 0;
    et->termine = 0;
    if (et->flags & TEXTE_FLAGS_SANSTHREAD || et->flags & TEXTE_FLAGS_TJSFOCUS)
       et->focus = 1;

    //cr�ation de la surface de "nettoyage"
    et->surfaceNettoyage = SDL_CreateRGBSurface(SDL_HWSURFACE, et->positionBlit.w, et->positionBlit.h, 32, 0, 0, 0, 0);
    if (!et->surfaceNettoyage)
       return TEXTE_ERREUR_SDL;
    if (et->typeBlit != TEXTE_BLIT_SHADED)
    {
       SDL_Rect positionZero = {0};
       SDL_BlitSurface(et->surfaceBlit, &et->positionBlit, et->surfaceNettoyage, &positionZero);
    }
    else SDL_FillRect(et->surfaceNettoyage, 0, SDL_MapRGB(et->surfaceNettoyage->format, et->couleurAP.r, et->couleurAP.g, et->couleurAP.b));

    //chargement de la surface de brush pour d�limiter l'espace de glisser-d�placer
    if (!TEXTE_brush)
        TEXTE_brush = IMG_Load("TEXTE_brush.png");
    if (et->flags & TEXTE_FLAGS_DEPLACER && !et->brush)
       et->brush = TEXTE_brush;
    if (!(et->flags & TEXTE_FLAGS_DEPLACER))
        et->brush = NULL;

    //cr�ation des s�maphores
    et->semaphoreE = SDL_CreateSemaphore(0);   //emission
    et->semaphoreR = SDL_CreateSemaphore(0);   //r�ception (confirmation)
    et->semaphoreA = SDL_CreateSemaphore(0);   //attente
    et->semTermine = SDL_CreateSemaphore(0);   //indique l'�tat de l'�dition
    if (!et->semaphoreE || !et->semaphoreR || !et->semaphoreA || !et->semTermine)
       return TEXTE_ERREUR_SDL;

    //cr�ation du tableau de positions des caract�res
    et->positionChar = malloc(sizeof(SDL_Rect) * et->limiteCaracteres);
    if (!et->positionChar)
       return TEXTE_ERREUR_ALLOC;
    memset(et->positionChar, 0, sizeof(SDL_Rect) * et->limiteCaracteres);

    //cr�ation des curseurs
    if (!TEXTE_curseurEdition)
       TEXTE_curseurEdition = TEXTE_CreerCurseur(TEXTE_dataCurseurEdition);
    if (!TEXTE_curseurDeplacement)
       TEXTE_curseurDeplacement = TEXTE_CreerCurseur(TEXTE_dataCurseurDeplacement);
    if (!TEXTE_curseurNormal)
       TEXTE_curseurNormal = SDL_GetCursor();

    #ifdef TEXTE_WIN32
    //cr�ation de la fen�tre pour le copier-coller
    if (!TEXTE_thread)
       TEXTE_thread = CreateThread(NULL, 0, ThreadFenetreCopiage, NULL, 0, NULL);
    #endif

    //cr�ation du thread
    et->thread = SDL_CreateThread(TEXTE_FonctionThread, et);
    if (!et->thread)
       return TEXTE_ERREUR_THREAD;

    //enregistrement de la nouvelle �dition
    if (!TEXTE_semaphoreTableau)
       TEXTE_semaphoreTableau = SDL_CreateSemaphore(1);
    else SDL_SemWait(TEXTE_semaphoreTableau);
    int i;
    for (i = 0 ; i < TEXTE_TAILLE_TABLEAU && TEXTE_tableau[i] ; i++);
    if (i < TEXTE_TAILLE_TABLEAU)
       TEXTE_tableau[i] = et;
    SDL_SemPost(TEXTE_semaphoreTableau);


    if ((et->flags & TEXTE_FLAGS_SANSTHREAD) == TEXTE_FLAGS_SANSTHREAD)  //si la fonction ne doit pas renvoyer imm�diatement
    {
       do  //boucle de relai des �v�nements
       {
          SDL_WaitEvent(&et->eventTampon);
          TEXTE_RelayerEvent(NULL, &et, 1);
       } while (!et->termine);
    }

    return 1;
}


int TEXTE_EnvoyerMessage(EDITIONTEXTE *et, int msg, int tempsMax)
{
    /*
    permet d'envoyer des messages � l'�dition correspondante.
    le message peut �tre TEXTE_MSG_MISEAJOUR pour mettre � jour l'�dition en fonction des �v�nements, TEXTE_MSG_MODIF quand vous modifiez un membre de la structure et souhaitez mettre � jour, ou TEXTE_MSG_TERMINER pour la terminer (ne pas utiliser SDL_TerminateThread()).
    tempsMax sp�cifie le temps maximum d'attente de la r�ponse de l'�dition, peut �tre TEXTE_TEMPS_INFINI
    */

    if (!et || !et->semaphoreE || !et->semaphoreR)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    if (msg)
       et->messageTampon = msg;

    SDL_SemPost(et->semaphoreR);  //envoi du message

    if (tempsMax < 0 || tempsMax == TEXTE_TEMPS_INFINI)  //attente de la r�ception
       SDL_SemWait(et->semaphoreE);
    else SDL_SemWaitTimeout(et->semaphoreE, tempsMax);

    return 1;
}

int TEXTE_RecevoirMessage(EDITIONTEXTE *et, int tempsMax)
{
    /*
    permet � une �dition d'attendre un message
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    if (!et || !et->semaphoreE || !et->semaphoreR)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    SDL_SemPost(et->semaphoreA);  //indique que l'�dition est en attente
    et->attente = 1;
    if (tempsMax <= 0 || tempsMax == TEXTE_TEMPS_INFINI) //attente du message
       SDL_SemWait(et->semaphoreR);
    else SDL_SemWaitTimeout(et->semaphoreR, tempsMax);
    SDL_SemTryWait(et->semaphoreA);  //indique que l'�dition n'est plus en attente
    et->attente = 0;

    SDL_SemPost(et->semaphoreE);   //validation de la r�ception
    return 1;
}

int TEXTE_RelayerEvent (SDL_Event *event, EDITIONTEXTE *et[], int nombreStructures)
{
    /*
    permet de relayer les �v�nements re�us dans le thread prinicipal � une ou plusieurs �ditions.
    *event peut �tre NULL, la fonction prendra alors le membre event de EDITIONTEXTE en remplacement.
    *et[] est un tableau de pointeurs sur les EDITIONTEXTE � relayer. Il peut �tre NULL, auquel cas la fonction enverra le message � toutes les �ditions enregistr�es.
    nombreStructures sp�cifie le nombre de pointeurs contenus dans le tableau *et[]. Non pris en compte si *et[] vaut NULL. Un nombre trop grand provoquera une erreur de segmentation.
    */


    if (!et)
    {
       et = TEXTE_tableau;
       if (nombreStructures <= 0)
          nombreStructures = TEXTE_TAILLE_TABLEAU;
       SDL_SemWait(TEXTE_semaphoreTableau);
    }
    else if (nombreStructures <= 0)
       nombreStructures = 1;

    int i, changerCurseur = 0;
    for (i = 0 ; i < nombreStructures ; i++)
    {
        if (et[i])
        {
            if (event)
               et[i]->eventTampon = *event;
            et[i]->eventTampon.button.x -= et[i]->positionEdition.x;   //recadrage de la souris
            et[i]->eventTampon.button.y -= et[i]->positionEdition.y;
            TEXTE_EnvoyerMessage(et[i], TEXTE_MSG_MISEAJOUR, 100);   //envoi du message de mise � jour

            //changement du curseur
            if (TEXTE_TestPositionSouris(et[i]->event, et[i]->positionBlit, 0, 0))
            {
               if (et[i]->flags & TEXTE_FLAGS_DEPLACER && et[i]->brush && TEXTE_TestPositionSouris(et[i]->event, et[i]->positionBlit, et[i]->brush->w, 0))
                  changerCurseur = 2;
               else changerCurseur = 1;
            }
        }
    }

    if (et == TEXTE_tableau)
       SDL_SemPost(TEXTE_semaphoreTableau);

    if (changerCurseur == 1 && TEXTE_curseurActuel != 1)
       SDL_SetCursor(TEXTE_curseurEdition);
    else if (changerCurseur == 2 && TEXTE_curseurActuel != 2)
         SDL_SetCursor(TEXTE_curseurDeplacement);
    else if (!TEXTE_curseurActuel)
         SDL_SetCursor(TEXTE_curseurNormal);
    TEXTE_curseurActuel = changerCurseur;

    return 1;
}

int TEXTE_ToutBlitter (EDITIONTEXTE *et[], int nombreStructures, SDL_Surface *ecran)
{
    /*
    cette fonction permet de blitter toutes les �ditions enregitr�es sur un �cran donn�.
    *et[] est un tableau de pointeurs, nombreStructures sp�cifie sa taille (voir fonction TEXTE_RelayerEvent()).
    ecran ne peut �tre NULL.
    */

    if (!ecran)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    if (!et)
    {
       et = TEXTE_tableau;
       if (nombreStructures <= 0)
          nombreStructures = TEXTE_TAILLE_TABLEAU;
       SDL_SemWait(TEXTE_semaphoreTableau);
    }
    else if (nombreStructures <= 0)
       nombreStructures = 1;

    int i;
    for (i = 0 ; i < nombreStructures ; i++)
    {
        if (et[i] && et[i]->surfaceBlit)
           SDL_BlitSurface(et[i]->surfaceBlit, NULL, ecran, &et[i]->positionEdition);  //blit de la surface � la position sp�cifi�e
    }

    if (et == TEXTE_tableau)
       SDL_SemPost(TEXTE_semaphoreTableau);

    return 1;
}

int TEXTE_ToutEffacer (EDITIONTEXTE *et[], int nombreStructures, int liberer)
{
    /*
    cette fonction permet de d�truire toutes les �ditions enregitr�es. La lib�ration de certaines zones de m�moires reste cependant � votre charge (voir TEXTE_CreerEditionEx()), sauf si vous sp�cifiez le param�tre "liberer" � 1.
    *et[] est un tableau de pointeurs, nombreStructures sp�cifie sa taille (voir fonction TEXTE_RelayerEvent()).
    */

    if (!et)
    {
       et = TEXTE_tableau;
       if (nombreStructures <= 0)
          nombreStructures = TEXTE_TAILLE_TABLEAU;
       SDL_SemWait(TEXTE_semaphoreTableau);
    }
    else if (nombreStructures <= 0)
       nombreStructures = 1;

    SDL_sem **tabSem = malloc(sizeof(SDL_sem*)*nombreStructures);
    memset(tabSem, 0, sizeof(SDL_sem*)*nombreStructures);

    int i;
    for (i = 0 ; i < nombreStructures ; i++)
    {
        if (et[i])
        {
           if (tabSem)
              tabSem[i] = et[i]->semTermine;   //enregistrement de la s�maphore t�moin
           TEXTE_EnvoyerMessage(et[i], TEXTE_MSG_TERMINER, 100);  //destruction de l'�dition

           if (liberer)  //lib�ration de certaines zones de m�moire
           {
              /*attention : tout pointeur non null parmi les membres texte, surfaceBlit et brush verra sa cible lib�r�e.
              Veuillez faire attention avant de passer les structures pour �viter des erreurs de segmentation.*/

              if (et[i]->surfaceBlit)
                 SDL_FreeSurface(et[i]->surfaceBlit);
              if (et[i]->texte)
                 free(et[i]->texte);
              if (et[i]->brush && et[i]->brush != TEXTE_brush)
                 SDL_FreeSurface(et[i]->brush);

              et[i]->surfaceBlit = NULL;
              et[i]->texte = NULL;
              et[i]->brush = NULL;
           }
        }
    }

    if (et == TEXTE_tableau)
       SDL_SemPost(TEXTE_semaphoreTableau);

    for (i = 0 ; tabSem && i < nombreStructures ; i++)   //attente de la fin de toutes les �ditions
    {
        if (tabSem[i])
            SDL_SemWait(tabSem[i]);
    }
    if (tabSem)
       free(tabSem);

    return 1;
}


int TEXTE_ToutAttendre (EDITIONTEXTE *et[], int nombreStructures)
{
    /*
    cette fonction permet au thread appelant d'�tre arr�t� jusqu'� ce que toutes les �ditions aient fini de traiter les messages.
    fournie uniquement pour tenter de r�duire le scintillement sur certaines machines.
    */

    if (!et)
    {
       et = TEXTE_tableau;
       if (nombreStructures <= 0)
          nombreStructures = TEXTE_TAILLE_TABLEAU;
       SDL_SemWait(TEXTE_semaphoreTableau);
    }
    else if (nombreStructures <= 0)
       nombreStructures = 1;

    int i;
    for (i = 0 ; i < nombreStructures ; i++)
    {
        if (et[i] && et[i]->semaphoreA)
        {
           SDL_SemWait(et[i]->semaphoreA);
           SDL_SemPost(et[i]->semaphoreA);
        }
    }

    if (et == TEXTE_tableau)
       SDL_SemPost(TEXTE_semaphoreTableau);

    return 1;
}

int TEXTE_FonctionThread (void *ptv)
{
    /*
    la fonction appel�e pour servir de thread s�par�
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    EDITIONTEXTE *et = ptv;
    et->codeRetour = TEXTE_Editer(et);  //appel de la fonction principale de l'�dition


    //fermeture de l'�dition
    et->termine = 1;
    SDL_SemPost(et->semTermine);

    //d�senregistrement de l'�dition
    SDL_SemWait(TEXTE_semaphoreTableau);
    int i;
    for (i = 0 ; i < TEXTE_TAILLE_TABLEAU && TEXTE_tableau[i] != et ; i++);
    if (i < TEXTE_TAILLE_TABLEAU)
       TEXTE_tableau[i] = NULL;
    for (; i+1 < TEXTE_TAILLE_TABLEAU ; i++)
        TEXTE_tableau[i] = TEXTE_tableau[i+1];
    if (i < TEXTE_TAILLE_TABLEAU)
       TEXTE_tableau[i] = NULL;
    SDL_SemPost(TEXTE_semaphoreTableau);

    //lib�ration des zones de m�moire allou�es automatiquement
    if (et->allocSurface && et->surfaceBlit)
    {
       SDL_FreeSurface(et->surfaceBlit);
       et->surfaceBlit = NULL;
    }
    if (et->surfaceNettoyage)
       SDL_FreeSurface(et->surfaceNettoyage);
    if (et->positionChar)
       free(et->positionChar);

    et->surfaceNettoyage = NULL;
    et->positionChar = NULL;


    //fermeture des s�maphores
    if (et->semaphoreR)
       SDL_DestroySemaphore(et->semaphoreR);
    if (et->semaphoreE)
       SDL_DestroySemaphore(et->semaphoreE);
    if (et->semaphoreA)
       SDL_DestroySemaphore(et->semaphoreA);

    return et->codeRetour; //1 si l'utilisateur a valid� le texte, 0 sinon.
}

int TEXTE_Editer (EDITIONTEXTE *et)
{
    /*
      la fonction principale d'une �dition : c'est elle qui r�ceptionne et traite les messages.
      fonction interne � la librairie, non recommand�e pour un usage externe
    */

    et->positionCurseur = 0;
    et->premierCaractere = 0;
    TEXTE_RedessinerEdition (et);   //premier dessin de l'�dition
    SDL_EnableUNICODE(1);  //autorisation de l'unicode pour le traitement des caract�res du clavier

    int continuer = 1,
        boutonSouris = 0,
        ctrl,
        maj = 0;

    while (continuer)
    {
          et->message = 0;
          boutonSouris = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1);   //pr�l�vement de l'�tat du bouton gauche de la souris
          Uint8 *etatClavier = SDL_GetKeyState(NULL);   //"photo" du clavier pour l'�tat des touches
          ctrl = etatClavier[SDLK_LCTRL];

          if (et->focus && boutonSouris && et->mouvement && et->positionCurseur == et->dernierCaractere && et->event.button.x >= et->positionChar[et->positionCurseur].x && et->texte[et->positionCurseur])
          {
             //pour faire d�filer le texte � droite en cas de s�lection prolong�e hors de la surface de l'�dition
             if (et->flags & TEXTE_FLAGS_MULTILIGNES)   //si l'�dition est multilignes, le calcul du dernier premier caract�re est difficile : on laisse la fonction s'en charger
             {
                et->positionCurseur++;
                TEXTE_AjusterPremierCaractere(et);
             }
             else    //sinon, une simple soustraction se charge de tout d�placer
             {
                 et->positionCurseur++;
                 et->premierCaractere++;
                 et->dernierCaractere++;
             }

             TEXTE_RedessinerEdition (et);  //mise � jour de l'�dition
             et->message = TEXTE_MSG_ECHAP;  //indique de ne pas traiter les messages
          }
          else if (et->focus && boutonSouris && et->mouvement && et->positionCurseur == et->premierCaractere && et->event.button.x <= et->positionChar[et->premierCaractere].x && et->premierCaractere > 0)
          {
               //pour faire d�filer le texte � gauche en cas de s�lection prolong�e hors de la surface de l'�dition
               if (et->flags & TEXTE_FLAGS_MULTILIGNES)
               {
                   et->premierCaractere = 0;
                   et->positionCurseur--;
                   TEXTE_AjusterPremierCaractere(et);
               }
               else
               {
                   et->positionCurseur--;
                   et->premierCaractere--;
                   et->dernierCaractere--;
               }

               TEXTE_RedessinerEdition (et);
               et->message = TEXTE_MSG_ECHAP;
          }
          else
          {
              //r�ception des messages
              TEXTE_RecevoirMessage(et, TEXTE_TEMPS_INFINI);
              et->event = et->eventTampon;  //les tampons servent � emp�cher une modification de l'event ou du message par un autre thread alors que celui-ci est en utilisation
              et->message = et->messageTampon;
          }

          //gestion de la touche MAJ pour une s�lection au clavier
          if (!maj && etatClavier[SDLK_LSHIFT])
          {
             maj = 1;
             et->debutSelectionInitiale = et->positionCurseur;
          }
          else if (maj && !etatClavier[SDLK_LSHIFT])
               maj = 0;

          et->mouvement = 0;
          switch (et->message)
          {
                 case TEXTE_MSG_ECHAP:  //message de break
                      break;
                 case TEXTE_MSG_TERMINER:  //terminer l'�dition
                      continuer = 0;
                      break;
                 case TEXTE_MSG_MODIF:   //reconsid�rer les nouveaux param�tres
                      et->positionCurseur = 0;
                      et->premierCaractere = 0;
                      TEXTE_RedessinerEdition(et);   //dessin de l'�dition
                      break;
                 case TEXTE_MSG_MISEAJOUR:  //mettre a jour l'�dition en fonction des nouveaux �v�nements
                      switch (et->event.type)
                      {
                             case SDL_MOUSEBUTTONDOWN:
                                  if (et->event.button.button != SDL_BUTTON_LEFT)  //seul le bouton gauche de la souris est g�r�
                                     break;

                                  if (TEXTE_TestPositionSouris(et->event, et->positionBlit, 0, 0))
                                  {
                                     //si un clic a �t� effectu� sur l'�dition, elle prend le focus clavier
                                     et->focus = 1;
                                     SDL_Delay(10);

                                     //si il y a conflit avec une autre �dition, c'est la plus r�cente qui l'emporte
                                     int i;
                                     SDL_SemWait(TEXTE_semaphoreTableau);
                                     for (i = TEXTE_TAILLE_TABLEAU - 1 ; et->focus && i >= 0 && TEXTE_tableau[i] != et ; i--)
                                     {
                                         if (TEXTE_tableau[i] && TEXTE_tableau[i]->focus && !(TEXTE_tableau[i]->flags & TEXTE_FLAGS_TJSFOCUS))
                                            et->focus = 0;
                                     }
                                     SDL_SemPost(TEXTE_semaphoreTableau);
                                     boutonSouris = 0;

                                     if (et->focus)
                                     {
                                        if (et->flags & TEXTE_FLAGS_DEPLACER && et->brush && TEXTE_TestPositionSouris(et->event, et->positionBlit, et->brush->w, 0))
                                        {
                                           //si un clic est effectu� dans la zone de cliquer-d�placer, on pr�pare le d�placement
                                           et->deplacement = 1;
                                           et->differenceAuClic.x = et->event.button.x;
                                           et->differenceAuClic.y = et->event.button.y;
                                        }
                                        else
                                        {
                                            TEXTE_CalculerPositionCurseur(et);   //calcul de la nouvelle position du curseur en fonction de celle de la souris
                                            et->debutSelectionInitiale = et->positionCurseur;
                                            et->finSelection = -1;  //destruction de toute s�lection de texte pr�c�demment effectu�e
                                        }
                                     }
                                  }
                                  else if (!(et->flags & TEXTE_FLAGS_TJSFOCUS))
                                  {
                                       //si un clic a �t� effectu� en dehors de l'�dition, elle perd le focus
                                       et->focus = 0;
                                       et->finSelection = -1;
                                  }

                                  TEXTE_RedessinerEdition (et);
                                  break;
                             case SDL_MOUSEMOTION:
                                  //pour la gestion du d�placement de l'�dition � la souris
                                  if (et->deplacement && !boutonSouris)
                                     et->deplacement = 0;
                                  if (et->deplacement)
                                  {
                                     et->positionEdition.x = et->event.button.x + et->positionEdition.x - et->differenceAuClic.x;
                                     et->positionEdition.y = et->event.button.y + et->positionEdition.y - et->differenceAuClic.y;
                                     break;
                                  }

                                  //pour la gestion de la s�lection du texte � la souris
                                  if (!et->focus || !boutonSouris)
                                     break;
                                  et->mouvement = 1;

                                  TEXTE_CalculerPositionCurseur(et);
                                  TEXTE_Selectionner(et);

                                  TEXTE_RedessinerEdition (et);
                                  break;
                             case SDL_KEYDOWN:
                                  if (!et->focus)   //aucune touche n'est trait�e si l'�dition n'a pas le focus
                                     break;

                                  switch (et->event.key.keysym.sym)
                                  {
                                         case SDLK_TAB:   //la touche TAB permet de passer le focus d'une �dition � une autre
                                         {
                                              if (et->flags & TEXTE_FLAGS_TJSFOCUS)
                                                 break;

                                              SDL_Delay(10);

                                              int i;
                                              SDL_SemWait(TEXTE_semaphoreTableau);
                                              for (i = 0 ; i < TEXTE_TAILLE_TABLEAU && TEXTE_tableau[i] != et ; i++);  //recherche de l'�dition actuelle
                                              i++;
                                              if (!TEXTE_tableau[i])
                                                 i = 0;
                                              if (TEXTE_tableau[i])
                                              {
                                                 TEXTE_tableau[i]->focus = 1;  //passage du focus � l'�dition suivante
                                                 et->focus = 0;
                                              }

                                              if (TEXTE_tableau[i])
                                              {
                                                 //mise � jour des deux �ditions concern�es
                                                 TEXTE_RedessinerEdition(TEXTE_tableau[i]);
                                                 TEXTE_RedessinerEdition(et);
                                              }
                                              SDL_SemPost(TEXTE_semaphoreTableau);

                                              break;
                                         }
                                         case SDLK_INSERT:  //ins�rer / remplacer le texte
                                              TEXTE_inserer = !TEXTE_inserer;
                                              break;
                                         case SDLK_KP_ENTER:
                                         case SDLK_ENTER:
                                              if (et->flags & TEXTE_FLAGS_TJSFOCUS && et->flags & TEXTE_FLAGS_LECTURESEULE)
                                                 break;

                                              if ((maj || ctrl) && et->flags & TEXTE_FLAGS_MULTILIGNES && !(et->flags & TEXTE_FLAGS_LECTURESEULE))
                                              {
                                                 TEXTE_InsererCaractere(et, '\n');  //pour passer � la ligne suivante dans une �dition multilignes (ENTER + CTRL ou MAJ)
                                                 TEXTE_AjusterPremierCaractere(et);  //ajustement du premier caract�re affich� pour �tre sur de faire rentrer le curseur dans la zone affich�e
                                              }
                                              else if (et->flags & TEXTE_FLAGS_TJSFOCUS)
                                                   continuer = 0;
                                              else et->focus = 0;
                                              break;
                                         case SDLK_ESCAPE:   //perte du focus ou fermeture de l'�dition selon les flags
                                              if (et->flags & TEXTE_FLAGS_TJSFOCUS)
                                                   continuer = 0;
                                              else et->focus = 0;
                                              break;
                                         case SDLK_HOME:   //remise au d�but du curseur
                                              et->positionCurseur = 0;
                                              et->premierCaractere = 0;
                                              et->finSelection = -1;
                                              break;
                                         case SDLK_END:   //envoi du curseur � la fin
                                              et->positionCurseur = strlen(et->texte);
                                              et->finSelection = -1;
                                              TEXTE_AjusterPremierCaractere(et);
                                              break;
                                         case SDLK_LEFT:   //d�placement du curseur sur la gauche
                                              if (et->positionCurseur > 0)
                                              {
                                                 int yInit = et->positionChar[et->positionCurseur].y;
                                                 et->positionCurseur--;

                                                 if (!maj)
                                                    et->finSelection = -1;  //si pas MAJ, destruction de toute s�lection
                                                 else TEXTE_Selectionner(et);  //si MAJ, modification de la s�lection en cons�quence

                                                 if (yInit != et->positionChar[et->positionCurseur].y && et->positionCurseur < et->premierCaractere)  //si le curseur sort de la zone et il y a changement de ligne
                                                 {
                                                    et->premierCaractere = 0;     //on remet tout � z�ro
                                                    TEXTE_AjusterPremierCaractere(et);
                                                 }
                                              }
                                              break;
                                         case SDLK_RIGHT:   //d�placement du curseur sur la droite
                                              if (et->texte[et->positionCurseur])
                                              {
                                                 et->positionCurseur++;
                                                 if (!maj)
                                                    et->finSelection = -1;
                                                 else TEXTE_Selectionner(et);
                                                 TEXTE_AjusterPremierCaractere(et);   //ajustement du premier caract�re pour faire rentrer le curseur dans la zone d'affichage

                                              }
                                              break;
                                         case SDLK_UP:
                                         {
                                              if (et->positionCurseur <= 0)
                                                 break;

                                              int i = et->positionCurseur,
                                                  yInit = et->positionChar[i].y,
                                                  xInit = et->positionChar[i].x + et->positionChar[i].w/2.0;

                                              for (i = et->positionCurseur-1 ; i >= 0 && et->positionChar[i].y == yInit ; i--);   //on monte d'une ligne
                                              yInit = et->positionChar[i].y;
                                              for (; i >= 0 && et->positionChar[i].y == yInit && et->positionChar[i].x > xInit ; i--);   //puis on recherche le caract�re � la m�me absicsse
                                              if (et->positionChar[i].y != yInit)
                                                 i++;

                                              et->positionCurseur = i;

                                              if (!maj)
                                                 et->finSelection = -1;  //si pas MAJ, destruction de toute s�lection
                                              else TEXTE_Selectionner(et);  //si MAJ, modification de la s�lection en cons�quence

                                              if (et->positionCurseur < et->premierCaractere)  //si le curseur sort de la zone
                                              {
                                                 et->premierCaractere = 0;     //on remet tout � z�ro
                                                 TEXTE_AjusterPremierCaractere(et);
                                              }
                                              break;
                                         }
                                         case SDLK_DOWN:
                                         {
                                              if (et->positionCurseur >= strlen(et->texte))
                                                 break;

                                              int i = et->positionCurseur,
                                                  yInit = et->positionChar[i].y,
                                                  xInit = et->positionChar[i].x - et->positionChar[i].w/2.0;

                                              for (i = et->positionCurseur+1 ; i < et->dernierCaractere && et->positionChar[i].y == yInit ; i++);   //on descend d'une ligne
                                              yInit = et->positionChar[i].y;
                                              for (; i < et->dernierCaractere && et->positionChar[i].y == yInit && et->positionChar[i].x < xInit ; i++);   //puis on recherche le caract�re � la m�me abscisse
                                              if (et->positionChar[i].y != yInit)
                                                 i--;

                                              et->positionCurseur = i;

                                              if (!maj)
                                                 et->finSelection = -1;  //si pas MAJ, destruction de toute s�lection
                                              else TEXTE_Selectionner(et);  //si MAJ, modification de la s�lection en cons�quence
                                              TEXTE_AjusterPremierCaractere(et);
                                              break;
                                         }
                                         case SDLK_BACKSPACE:  //supprimer le caract�re pr�c�dant le curseur
                                         case SDLK_DELETE:   //supprimer le caractere suivant le curseur
                                              if (et->flags & TEXTE_FLAGS_LECTURESEULE)
                                                 break;

                                              if (et->event.key.keysym.sym == SDLK_DELETE || et->finSelection > 0)
                                              {
                                                  if (et->finSelection <= 0 && et->texte[et->positionCurseur])
                                                     strcpy(et->texte + et->positionCurseur, et->texte + et->positionCurseur + 1);
                                                  else TEXTE_SupprimerSelection(et);  //s'il y a une s�lection, la supprimer en entier
                                                  break;
                                              }

                                              //event.key.keysym.sym == SDLK_BACKSPACE
                                              TEXTE_InsererCaractere(et, '\b');
                                              if (et->premierCaractere > 0)
                                              {
                                                 et->premierCaractere--;
                                                 TEXTE_AjusterPremierCaractere(et);
                                              }
                                              break;
                                         default:   //entr�e d'un caract�re standard
                                         {
                                                 if (ctrl)  //codes de copier-coller (CTRL+C ; CTRL+X ; CTRL+V) et de s�lection (CTRL+A)
                                                 {
                                                    //pour des raisons techniques, ceci ne peut fonctionner que sur un clavier AZERTY fran�ais
                                                    switch (et->event.key.keysym.sym)
                                                    {
                                                           case 'x':  //coupage
                                                                if (et->flags & TEXTE_FLAGS_LECTURESEULE)
                                                                   break;
                                                           case 'c':  //copiage
                                                                if (et->finSelection <= 0)  //ne pas ex�cuter si aucune s�lection
                                                                   break;

                                                                strncpy(TEXTE_copiage, et->texte + et->debutSelection, et->finSelection - et->debutSelection);  //envoi de la s�lection au buffer global de copiage
                                                                TEXTE_copiage[et->finSelection - et->debutSelection] = '\0';
                                                                #ifdef TEXTE_WIN32
                                                                SetDlgItemTextA(TEXTE_fenetre, TEXTE_IDEDIT, TEXTE_copiage);
                                                                SendMessageA(GetDlgItem(TEXTE_fenetre, TEXTE_IDEDIT), EM_SETSEL, 0, -1);
                                                                SendMessageA(GetDlgItem(TEXTE_fenetre, TEXTE_IDEDIT), WM_COPY, 0, 0);
                                                                #endif

                                                                if (et->event.key.keysym.sym == 'x')   //en cas de coupage, supprimer la s�lection apr�s copie
                                                                   TEXTE_SupprimerSelection(et);
                                                                break;
                                                           case 'v':   //collage
                                                                #ifdef TEXTE_WIN32
                                                                SetDlgItemTextA(TEXTE_fenetre, TEXTE_IDEDIT, "");
                                                                SendMessageA(GetDlgItem(TEXTE_fenetre, TEXTE_IDEDIT), WM_PASTE, 0, 0);
                                                                GetDlgItemTextA(TEXTE_fenetre, TEXTE_IDEDIT, TEXTE_copiage, TEXTE_MAX_CARACTERES);
                                                                #endif

                                                                if (!TEXTE_copiage[0])   //ne pas ex�cuter si le buffer de copiage est vide
                                                                   break;

                                                                if (et->finSelection > 0)
                                                                {
                                                                   //si s�lection, la supprimer avant d'ins�rer le presse-papier
                                                                   et->finSelection -= !TEXTE_inserer;
                                                                   TEXTE_SupprimerSelection(et);
                                                                }

                                                                int i;
                                                                for (i = 0 ; i < et->limiteCaracteres && TEXTE_copiage[i] && (TEXTE_copiage[i] != '\n' || et->flags & TEXTE_FLAGS_MULTILIGNES) ; i++)  //ins�rer tout le buffer de copiage dans l'�dition
                                                                    TEXTE_InsererCaractere(et, TEXTE_TrierCaractere(et, TEXTE_copiage[i]));

                                                                TEXTE_AjusterPremierCaractere(et);   //ajustement du premier caract�re pour s'assurer que tout est affich�
                                                                break;
                                                           case 'q':   //s�l�ctionner tout
                                                                et->debutSelectionInitiale = 0;
                                                                et->debutSelection = 0;
                                                                et->finSelection = strlen(et->texte);
                                                                break;
                                                           default:
                                                                break;
                                                    }
                                                    break;
                                                 }


                                                 if (!et->event.key.keysym.unicode || et->flags & TEXTE_FLAGS_LECTURESEULE)  //si le caract�re n'est pas reconnu ou si l'�dition est en lecture seule, on breake.
                                                    break;

                                                 char nouveauCaractere;
                                                 if (et->event.key.keysym.unicode < 255)
                                                    nouveauCaractere = et->event.key.keysym.unicode;
                                                 else nouveauCaractere = '?';  //si un caract�re n'est pas ASCII, on met un '?'
                                                 //RQ : l'exemple fourni par la SDL n'accepte pas les accents...

                                                 nouveauCaractere = TEXTE_TrierCaractere(et, nouveauCaractere);
                                                 if (!nouveauCaractere)
                                                    break;

                                                 if (et->finSelection > 0)
                                                 {
                                                    //si s�lection, la supprimer avant d'ins�rer le caract�re
                                                    et->finSelection -= !TEXTE_inserer;
                                                    TEXTE_SupprimerSelection(et);
                                                 }

                                                 TEXTE_InsererCaractere(et, nouveauCaractere);  //insertion du caract�re
                                                 TEXTE_AjusterPremierCaractere(et);   //ajustement du premier caract�re pour s'assurer que le nouveau est bien affich�

                                                 break;
                                         }
                                  }
                                  TEXTE_RedessinerEdition (et);  //mise � jour de l'�dition
                                  break;
                      }
                      break;
          }
    }

    if (et->event.key.keysym.sym == SDLK_ESCAPE)
       return 0;   //la fonction renvoie 0 si l'utilisateur a press� ECHAP mais ne modifie pas le buffer de texte
    else return 1;
}



int TEXTE_RedessinerEdition (EDITIONTEXTE *et)
{
    /*
    fonction de dessin d'une �dition
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    //v�rification des param�tres en entr�e (cette fonction �tant appel�e uniquement par la fonction principale d'une �dition TEXTE_Editer(), la v�rification est moins pouss�e qu'elle ne devrait �tre)
    if (!et || !et->police || !et->positionChar || !et->surfaceBlit || !et->texte)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    //ajustement des param�tres
    if (et->limiteCaracteres <= 0)
       et->limiteCaracteres = TEXTE_MAX_CARACTERES;
    if (et->positionBlit.w <= 0)
       et->positionBlit.w = et->surfaceBlit->w - et->positionBlit.x;
    if (et->positionBlit.h <= 0)
       et->positionBlit.h = et->surfaceBlit->h - et->positionBlit.y;

    if (et->premierCaractere < 0)
       et->premierCaractere = 0;
    if (et->premierCaractere >= et->limiteCaracteres)
       et->premierCaractere = et->limiteCaracteres - 1;
    if (et->positionCurseur < 0)
       et->positionCurseur = 0;
    if (et->positionCurseur >= et->limiteCaracteres)
       et->positionCurseur = et->limiteCaracteres - 1;

    //nettoyage de la surface de blit
    SDL_BlitSurface(et->surfaceNettoyage, NULL, et->surfaceBlit, &et->positionBlit);

    //nettoyage du texte et des positions des caracteres
    memset(et->texte + strlen(et->texte), 0, et->limiteCaracteres - strlen(et->texte));
    memset(&(et->positionChar[et->dernierCaractere]), 0, sizeof(SDL_Rect) * (et->limiteCaracteres - et->dernierCaractere));

    //pr�parations de variables locales
    SDL_Surface *surfaceTexte = NULL;
    SDL_Rect positionTexte = {0};  //servira � limiter le blit d'un texte aux dimensions sp�cifi�es par l'utilisateur, ainsi on est s�r de ne pas d�border
    positionTexte.w = et->positionBlit.w;
    positionTexte.h = et->positionBlit.h;

    SDL_SemWait(TEXTE_semTTF);  //voir la d�claration de TEXTE_semTTF
    et->hauteurMaxi = TEXTE_HauteurMaxi(et->police, NULL);
    SDL_SemPost(TEXTE_semTTF);

    SDL_Surface *surfaceCurseur = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, et->hauteurMaxi, 32, 0, 0, 0, 0);  //la surface repr�sentant le curseur dans l'�dition, une simple barre de 1 px de large
    if (surfaceCurseur)
       SDL_FillRect(surfaceCurseur, NULL, SDL_MapRGB(surfaceCurseur->format, et->couleurPP.r, et->couleurPP.g, et->couleurPP.b));

    int i, j, continuer = 1, w = 0, h = 0;
    memset(et->positionChar, -1, sizeof(SDL_Rect) * et->premierCaractere);   //remise � z�ro de toutes les positions des caract�res situ�s avant le premier affich�

    char tampon[2] = {0};   //un tampon pour l'affichage d'un unique caract�re
    int premierCaractereLigne = et->premierCaractere,   //enregistre le num�ro de premier caractere de la ligne en cours (pour mise en forme)
        changementLigne = 0,
        positionInit = et->positionBlit.x;   //position de d�but de ligne

    if (et->flags & TEXTE_FLAGS_DEPLACER && et->brush)
       positionInit += et->brush->w + 5;

    et->positionChar[et->premierCaractere].x = positionInit;  //ajustement de la position du premier caract�re affich�
    et->positionChar[et->premierCaractere].y = et->positionBlit.y;

    if (et->texteFixe[0])
    {
       TTF_SizeText(et->police, et->texteFixe, &w, NULL);  //r�cup�ration de la taille du texteFixe
       et->positionChar[et->premierCaractere].x += w;  //ajustement de la position du premier caract�re affich�
    }


    for (i = et->premierCaractere ; i < et->limiteCaracteres && et->texte[i] && continuer ; i++)  //boucle de d�finition des positions des lettres
    {
        tampon[0] = et->texte[i];
        if (et->flags & TEXTE_FLAGS_MOTDEPASSE)
           tampon[0] = '*';

        SDL_SemWait(TEXTE_semTTF);
        TTF_SizeText(et->police, tampon, &w, &h);  //r�cup�ration de la taille du caract�re � afficher
        SDL_SemPost(TEXTE_semTTF);

        if (w >= et->positionBlit.w)  //si le caract�re ne rentre pas en largeur, il est inutile d'insister...
           continuer = 0;

        if (et->texte[i] != '\n' && et->texte[i] != '\t' && et->texte[i] != '\r' && et->texte[i] != '\b')  //les caract�res de mise en forme sont refus�s
        {
           et->positionChar[i].w = w;  //stockage des dimensions du caract�re
           et->positionChar[i].h = h;
        }
        else
        {
            et->positionChar[i].w = 0;  //effacement des dimensions du caract�re
            et->positionChar[i].h = 0;
        }

        if (et->texte[i] == '\n' || (et->positionChar[i].x + et->positionChar[i].w >= et->positionBlit.x + et->positionBlit.w))
        {
           if (et->flags & TEXTE_FLAGS_MULTILIGNES)
           {
              if (et->texte[i] != '\n')
              {
                 for (j = i-1 ; j >= 0 && et->texte[j] != ' ' && et->texte[j] != '\n' ; j--);  //recherche de l'espace le plus proche pour effectuer un passage � la ligne
                 if (et->texte[j] == ' ')
                    i = j+1;
              }

              et->positionChar[i].x = positionInit;  //ajustement de la position du premier caract�re affich�
              et->positionChar[i].y += et->hauteurMaxi;    //changement de ligne
              if (et->positionChar[i].y + et->hauteurMaxi>= et->positionBlit.y + et->positionBlit.h)   //si d�passement hors de la surface autoris�e
                 continuer = 0;
           }
           else
           {
               if (et->texte[i] == '\n')
                  et->texte[i] = '\0';  //si l'�dition n'est pas multi-lignes, aucun '\n' ne peut �tre accept�, ils sont consid�r�s comme des '\0'
               continuer = 0;
           }

           changementLigne = 1;
        }

        if ((et->flags & TEXTE_FLAGS_CENTRER || et->flags & TEXTE_FLAGS_ALIGNDROITE) && (!et->texte[i+1] || i+1 >= et->limiteCaracteres || changementLigne))
        {
           //centrage/alignement de la ligne
           int ajustement = et->positionBlit.x + et->positionBlit.w - et->positionChar[i-changementLigne].x - et->positionChar[i-changementLigne].w;
           if (et->flags & TEXTE_FLAGS_CENTRER)
              ajustement /= 2.0;
           for (j = premierCaractereLigne ; j < i+!changementLigne ; j++)    //si changementLigne=0, c'est que le caractere num�ro i est encore sur la m�me ligne, on le prend donc en compte...
               et->positionChar[j].x += ajustement;
           changementLigne = 0;
           premierCaractereLigne = i-changementLigne;
        }

        if (i+1 < et->limiteCaracteres)
        {
           //pr�paration de la position du prochain caract�re
           et->positionChar[i+1].x = et->positionChar[i].x + et->positionChar[i].w;
           et->positionChar[i+1].y = et->positionChar[i].y;
        }
    }
    et->dernierCaractere = i;  //enregistrement du dernier caract�re, il s'agit toujours d'un caract�re non affich�
    if (!continuer)
       et->dernierCaractere--;

    //ajout du texte fixe
    int flags = et->flags;
    if (et->flags & TEXTE_FLAGS_MOTDEPASSE)
       et->flags -= TEXTE_FLAGS_MOTDEPASSE;   //pour �viter l'affichage du texteFixe avec plein d'�toiles
    if (et->texteFixe[0] && TEXTE_Rendu(et, et->texteFixe, &surfaceTexte, 0) > 0)   //rendu du texte fixe
    {
       SDL_Rect positionBlit = et->positionChar[et->premierCaractere];
       if (et->flags & TEXTE_FLAGS_CENTRER && !et->texte[et->premierCaractere])   //centrage �ventuel du texte fixe
          positionBlit.x = positionInit + (et->positionBlit.w - surfaceTexte->w)/2.0;
       else if (et->flags & TEXTE_FLAGS_ALIGNDROITE && !et->texte[et->premierCaractere])   //alignement �ventuel du texte fixe
          positionBlit.x = positionInit + et->positionBlit.w - surfaceTexte->w;
       else positionBlit.x -= surfaceTexte->w;
       SDL_BlitSurface(surfaceTexte, &positionTexte, et->surfaceBlit, &positionBlit);   //blit du texte fixe
    }
    et->flags = flags;

    for (i = et->premierCaractere ; i < et->dernierCaractere ; i++)  //ajout des lettres sur la surface
    {
        tampon[0] = et->texte[i];
        if (surfaceTexte)
           SDL_FreeSurface(surfaceTexte);
        surfaceTexte = NULL;

        if (et->positionChar[i].w > 0)
        {
            if (TEXTE_Rendu(et, tampon, &surfaceTexte, i >= et->debutSelection && i < et->finSelection) > 0)
               SDL_BlitSurface(surfaceTexte, &positionTexte, et->surfaceBlit, &et->positionChar[i]);
        }
    }


    if (surfaceCurseur && et->focus)
    {
       //ajout du curseur sur la surface
       SDL_Rect positionBlit = et->positionChar[et->positionCurseur];
       if (et->flags & TEXTE_FLAGS_ALIGNDROITE && (positionBlit.x <= positionInit || (et->positionCurseur == et->premierCaractere && !et->texte[et->premierCaractere])))
          positionBlit.x = positionInit + et->positionBlit.w - 1;   //alignement �ventuel du curseur
       if (et->flags & TEXTE_FLAGS_CENTRER)   //centrage �ventuel du curseur
       {
          if (positionBlit.x <= positionInit)
             positionBlit.x = positionInit + et->positionBlit.w/2.0;
          else if (et->positionCurseur == et->premierCaractere && !et->texte[et->premierCaractere])
               positionBlit.x = positionInit + (et->positionBlit.w - surfaceTexte->w)/2.0 + surfaceTexte->w;
       }
       if (positionBlit.x >= et->positionBlit.x + et->positionBlit.w)
          positionBlit.x = et->positionBlit.x + et->positionBlit.w -1;
       SDL_BlitSurface(surfaceCurseur, &positionTexte, et->surfaceBlit, &positionBlit);
       SDL_FreeSurface(surfaceCurseur);  //lib�ration de la surface allou�e pour le curseur
       surfaceCurseur = NULL;
    }

    if (et->flags & TEXTE_FLAGS_DEPLACER)   //si possibilit� de d�placer l'�dition, ajout du brush indiquant la zone de glisser-d�placer
       TEXTE_AjouterBrush (et);


    SDL_Flip(et->surfaceBlit);   //mise � jour de la surface. Ajout�e uniquement pour le cas o� il s'agit de l'�cran.
    return 1;
}

int TEXTE_Rendu (EDITIONTEXTE *et, char texte[], SDL_Surface **surfaceTexte, int selectionne)
{
    /*
    cette fonction permet le rendu d'une cha�ne de caract�res en tenant compte de tous les param�tres de l'�dition
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    if (!texte || !surfaceTexte || !et || !et->police)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    SDL_Color couleurPP = et->couleurPP,
              couleurAP = et->couleurAP;
    int typeBlit = et->typeBlit;

    if (selectionne)  //si le rendu est demand� pour un texte s�lectionn�, on change les couleurs
    {
       couleurPP = et->couleurSelPP;
       couleurAP = et->couleurSelAP;
       et->typeBlit = TEXTE_BLIT_SHADED;
    }

    char *copieTexte = malloc(strlen(texte)+1);  //cr�ation d'un buffer dans lequel sera copi� le texte � afficher
    if (!copieTexte)
       return TEXTE_ERREUR_ALLOC;
    memset(copieTexte, 0, strlen(texte)+1);

    if (et->flags & TEXTE_FLAGS_MOTDEPASSE)  //copiage du texte dans le buffer...
       memset(copieTexte, '*', strlen(texte));   //... avec des �toiles s'il s'agit d'un mot de passe
    else strcpy(copieTexte, texte);

    SDL_SemWait(TEXTE_semTTF);  //voir la d�claration de TEXTE_semTTF
    switch (et->typeBlit)  //rendu du texte
    {
           case TEXTE_BLIT_SHADED:
                *surfaceTexte = TTF_RenderText_Shaded(et->police, copieTexte, couleurPP, couleurAP);
                break;
           case TEXTE_BLIT_BLENDED:
                *surfaceTexte = TTF_RenderText_Blended(et->police, copieTexte, couleurPP);
                break;
           default:
                   *surfaceTexte = TTF_RenderText_Solid(et->police, copieTexte, couleurPP);
                   break;
    }
    SDL_SemPost(TEXTE_semTTF);

    free(copieTexte);  //lib�ration du buffer de copie
    copieTexte = NULL;
    et->typeBlit = typeBlit;

    if (!*surfaceTexte)
       return TEXTE_ERREUR_SDL;
    else return 1;
}



int TEXTE_TestPositionSouris(SDL_Event event, SDL_Rect position, int largeur, int hauteur)
{
    /*
    fonction destin�e � d�terminer si la souris se trouve dans le rectangle pass�
    si largeur ou hauteur est <= 0, les membres w et h de position sont pris en compte
    */

    if (largeur <= 0)
       largeur = position.w;
    if (hauteur <= 0)
       hauteur = position.h;

    return event.button.x >= position.x && event.button.x <= position.x + largeur && event.button.y >= position.y && event.button.y <= position.y + hauteur;
}

int TEXTE_CalculerPositionCurseur(EDITIONTEXTE *et)
{
    /*
    cette fonction d�termine la position du curseur dans une �dition en fonction de celle de la souris
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    if (!et || !et->positionChar)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    if (et->dernierCaractere <= 0)
    {
       et->positionCurseur = 0;
       return 1;
    }

    if (et->dernierCaractere >= et->limiteCaracteres)
       et->dernierCaractere = et->limiteCaracteres - 1;

    int i,
        ligne = et->positionChar[et->premierCaractere].y,
        continuer = 1;

    if (et->flags & TEXTE_FLAGS_MULTILIGNES)  //recherche de la ligne sur laquelle est pos�e la souris dans une �dition multi-lignes
    {
        for (i = 0 ; continuer && i < et->dernierCaractere ; i++)  //on va de caract�re en caract�re jusqu'� ce que le caract�re soit situ� en dessous de la souris
        {
            if (et->event.button.y < et->positionChar[i].y)
            {
               if (i > 0)
                  i--;
               ligne = et->positionChar[i].y;
               continuer = 0;
            }
        }
        if (continuer)  //le curseur est sur la derni�re ligne
           ligne = et->positionChar[i-1].y;
    }

    for (i = et->premierCaractere ; i < et->dernierCaractere && ligne != et->positionChar[i].y ; i++);  //recherche du premier caract�re sur la ligne trouv�e

    continuer = 1;
    //recherche du caract�re le plus proche de la souris sur la ligne trouv�e
    for (; i < et->dernierCaractere && ligne == et->positionChar[i].y && continuer ; i++)   //on va de caract�re en caract�re jusqu'� ce que la souris soit dans la bonne zone
    {
        if (et->event.button.x <= et->positionChar[i].x + et->positionChar[i].w/2.0)
        {
           et->positionCurseur = i;
           continuer = 0;
        }
    }
    if (continuer && (i == strlen(et->texte) || (i == et->dernierCaractere && et->mouvement))) //le curseur est tout � la fin
       et->positionCurseur = i;
    else if (continuer)  //le curseur est � la fin de la ligne
         et->positionCurseur = i-1;

    return 1;
}

int TEXTE_InsererCaractere(EDITIONTEXTE *et, char caractere)
{
    /*
    permet d'ins�rer un unique caract�re ou un backspace dans une �dition (sans en refaire l'affichage)
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    if (!et || !et->texte || !caractere || caractere == '\t' || caractere == '\r')
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    if (et->positionCurseur < 0)
       et->positionCurseur = 0;
    if (et->positionCurseur >= et->limiteCaracteres - TEXTE_inserer && caractere != '\b')
       return TEXTE_ERREUR_OVERFLOW;

    if (caractere == '\b' && et->positionCurseur > 0)
    {
       //si backspace, il s'agit d'effacer le caract�re pr�c�dant le curseur puis de d�caler toute la cha�ne vers la "gauche"
       strcpy(et->texte + et->positionCurseur - 1, et->texte + et->positionCurseur);
       et->positionCurseur--;
    }
    else if (caractere != '\b')
    {
        if (TEXTE_inserer && et->texte[et->positionCurseur])
        {
           //si insertion, il faut d�caler toute la cha�ne vers la "droite" avant d'ajouter le caract�re
           char *tampon = malloc(strlen(et->texte + et->positionCurseur)+1);
           if (!tampon)
              return TEXTE_ERREUR_ALLOC;
           strcpy(tampon, et->texte + et->positionCurseur);
           strcpy(et->texte + et->positionCurseur + 1, tampon);
           free(tampon);
           tampon = NULL;
        }
        et->texte[et->positionCurseur] = caractere;  //ajout du caract�re
        et->positionCurseur++;
    }

    return 1;
}

int TEXTE_AjusterPremierCaractere(EDITIONTEXTE *et)
{
    /*
    cette fonction permet d'ajuster l'affichage d'une �dition pour permettre au curseur de rentrer dans la zone d'affichage
    fonction interne � la librairie, non recommand�e pour un usage externe
    par ailleurs, elle tr�s lente : elle utilise une des plus grosses fonctions de la libraire, en boucle !
    */

    int retour = TEXTE_RedessinerEdition(et);
    while (retour > 0 && et->positionCurseur >= et->dernierCaractere-1 && et->texte[et->dernierCaractere])
    {
       //tant que le curseur ne rentre pas, on incr�mente le premier caract�re pour d�caler le texte vers la gauche
       et->premierCaractere++;
       retour = TEXTE_RedessinerEdition(et);
    }

    return retour;
}

int TEXTE_Selectionner(EDITIONTEXTE *et)
{
    /*
    permet de modifier la s�lection actuelle en fonction de la position du curseur et de l'endroit o� la s�lection a commenc�
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    if (!et)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    if (et->positionCurseur < 0)
       et->positionCurseur = 0;

    if (et->positionCurseur > et->debutSelectionInitiale)  //debutSelection est toujours inf�rieur � finSelection
    {
       et->finSelection = et->positionCurseur;
       et->debutSelection = et->debutSelectionInitiale;
    }
    else
    {
        et->debutSelection = et->positionCurseur;
        et->finSelection = et->debutSelectionInitiale;
    }

    return 1;
}

int TEXTE_SupprimerSelection(EDITIONTEXTE *et)
{
    /*
    supprime la s�lection courante dans une �dition
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    if (!et || !et->texte || et->finSelection <= 0)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    strcpy(et->texte + et->debutSelection, et->texte + et->finSelection);
    memset(et->texte + strlen(et->texte), 0, et->limiteCaracteres - strlen(et->texte));  //mise � z�ro de tous les caract�res situ�s apr�s le '\0'
    et->positionCurseur = et->debutSelection;
    et->finSelection = -1;

    et->premierCaractere = 0;
    TEXTE_AjusterPremierCaractere(et);  //on r�ajuste toute l'�dition

    return 1;
}

char TEXTE_TrierCaractere(EDITIONTEXTE *et, char caractere)
{
    /*
    permet de refuser ou d'accepter un caact�re tap�, selon les flags de l'�dition
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    if (!et || !et->texte)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    if (et->flags & TEXTE_FLAGS_LECTURESEULE)
       return 0;

    if (!(et->flags & TEXTE_FLAGS_MULTILIGNES) && caractere == '\n')
       return 0;

    if (et->flags & TEXTE_FLAGS_SLMTNOMBRES)
    {
       //refus de tout autre caract�re qu'un nombre, � part un point ou une virgule en une seule occurence
       if (caractere == ',')
          return '.';
       if ((caractere < '0' || caractere > '9') && caractere != '.')
          return 0;
       if (caractere == '.' && strchr(et->texte, '.'))
          return 0;
    }

    return caractere;
}



int TEXTE_AjouterBrush (EDITIONTEXTE *et)
{
    /*
    permet d'ajouter � une �dition le brush signalant l'emplacement de la zone de cliquer-d�placer
    fonction interne � la librairie, non recommand�e pour un usage externe
    */

    if (!et || !et->brush || !et->surfaceBlit)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    SDL_Rect position = {0};
    for (; position.y + et->brush->h < et->positionBlit.h ; position.y += et->brush->h)
        SDL_BlitSurface(et->brush, NULL, et->surfaceBlit, &position);

    SDL_Rect position2 = {0};
    position2.w = et->brush->w;
    position2.h = et->positionBlit.h - position.y;
    SDL_BlitSurface(et->brush, &position2, et->surfaceBlit, &position);

    return 1;
}


SDL_Cursor* TEXTE_CreerCurseur(const char *image[])   //cr�� un curseur � partir du format XPM. Fourni par la doc de la SDL.
{
  int i, row, col;
  Uint8 data[4*16];
  Uint8 mask[4*16];
  int hot_x, hot_y;

  i = -1;
  for ( row=0; row<16; ++row ) {
    for ( col=0; col<16; ++col ) {
      if ( col % 8 ) {
        data[i] <<= 1;
        mask[i] <<= 1;
      } else {
        ++i;
        data[i] = mask[i] = 0;
      }
      switch (image[4+row][col]) {
        case '+':
          data[i] |= 0x01;
          mask[i] |= 0x01;
          break;
        case '.':
          mask[i] |= 0x01;
          break;
        case ' ':
          break;
      }
    }
  }
  sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
  return SDL_CreateCursor(data, mask, 16, 16, hot_x, hot_y);
}

int TEXTE_ModifierCurseurDefaut(SDL_Cursor* curseur)   //permet de modifier le curseur "normal" de la librairie (au cas o� celui-ci aurait chang�...)
{
    if (!curseur)
        return 0;

    TEXTE_curseurNormal = curseur;
    return 1;
}



#ifdef TEXTE_WIN32
LRESULT CALLBACK TEXTE_CallbackFenetre (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TEXTE_fenetre = hwnd;
    ShowWindow(hwnd, SW_HIDE);
    if (msg == WM_DESTROY)
       EndDialog(hwnd, 0);
    return FALSE;
}

DWORD WINAPI ThreadFenetreCopiage(LPVOID lpParameter)
{
   return DialogBoxA(GetModuleHandle(NULL), "TEXTE_boiteCopiage", NULL, (DLGPROC)TEXTE_CallbackFenetre);
}
#endif

