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

#include "main.h"


COLORREF couleurTrace = 0x000000FF,
         couleurRepere = 0x00FF0000,
         couleurAnnexe;
HWND carreCouleurTrace_Wnd,
     carreCouleurRepere_Wnd,
     carreCouleur_Wnd,
     optionsTrace_Wnd,
     optionsParam_Wnd,
     optionsAlgo_Wnd,
     optionsFonctions_Wnd,
     optionsVariables_Wnd,
     rappels_Wnd,
     listeFonctions_Wnd,
     listeVariables_Wnd,
     listeRappelsFonctions_Wnd,
     listeRappelsVariables_Wnd,
     fenetreOptions_Wnd;
HFONT policeSpeciale;
int derniereEditionRepere = 0,
    derniereSelectionListeOptions = 0,
    selectionPrecedente = 0,
    timerActif = 0;
SDL_TimerID timerId;
HICON iconeVide;

SDL_Surface *surfaceCourbe = NULL;

FonctionDefaut *tabFonctionsDefaut;
VariableDefaut *tabVariablesDefaut;
Fonction **tabFonctions = NULL;
Variable **tabVariables = NULL;

Bouton boutonApparition,
       boutonDisparition;

OptionsTrace optionsTrace;
OptionsParam optionsParam[4],
             optionsParamTemp[4];
OptionsAlgo optionsAlgo;

Menu menu[50];
Menu *menuActuel = NULL,
     *ancienMenu = NULL;

const char *dataCurseurCroix[] = {   //données XPM pour construire les curseurs
      "16 16 3 1",
      " 	c None",
      ".	c #FFFFFF",
      "+	c #000000",
      "                ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      " ......+......  ",
      " +++++++++++++  ",
      " ......+......  ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "      .+.       ",
      "                ",
      "                ",
      "7,7"
};
SDL_Cursor *curseurCroix,
           *curseurNormal;


int main(int argc, char *argv[])
{
    InitialiserVariables();

    SDL_Surface *ecran = NULL,
                *surfaceBoutons = NULL;
    SDL_Event event;
    srand(time(NULL));

    ecran = initSDL(LARGEUR, HAUTEUR, "Equaminator");

    if (TTF_Init() == -1) //Démarrage de SDL_ttf. Si erreur alors...
    {
       fprintf(stderr, "Erreur d'initialisation de TTF_Init : %s\n", TTF_GetError());
       exit(EXIT_FAILURE);
    }

    curseurCroix = TEXTE_CreerCurseur(dataCurseurCroix);
    curseurNormal = SDL_GetCursor();

    SDL_WM_SetIcon(SDL_LoadBMP("sdl_icone.bmp"), NULL);

    SDL_Surface *imageDeFond = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, HAUTEUR, 32, 0, 0, 0, 0),
                *surfaceAdditionelle = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, HAUTEUR, 32, 0, 0, 0, 0),
                *degrade = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, 1, 32, 0, 0, 0, 0),
                *secondCurseur = IMG_Load("curseur2.gif");
    SDL_SetColorKey(surfaceAdditionelle, SDL_SRCCOLORKEY, SDL_MapRGB(surfaceAdditionelle->format, 0, 0, 0));
    SDL_Rect positionFond = {0},
             position,
             positionSecondCurseur;
    POINT pointZoomCarre1 = {0},
          pointZoomCarre2 = {0};
    TTF_Font *courrierNew = TTF_OpenFont("cour.ttf", 13);

    int i;
    for (i = 0 ; i <= 255 ; i++)
    {
        SDL_FillRect(degrade, NULL, SDL_MapRGB(ecran->format, 255 - i, 255, 0));
        position.x = 0;
        position.y = i;
        SDL_BlitSurface(degrade, NULL, imageDeFond, &position);
    }

    for (i = 1 ; i <= 255 ; i++)
    {
        SDL_FillRect(degrade, NULL, SDL_MapRGB(ecran->format, i, 255, 0));
        position.x = 0;
        position.y = i + 255;
        SDL_BlitSurface(degrade, NULL, imageDeFond, &position);
    }
    //SDL_FillRect(imageDeFond, NULL, SDL_MapRGB(ecran->format, 255, 255, 255));
    SDL_BlitSurface(imageDeFond, NULL, ecran, &positionFond);


    //création des entrées de texte
    EDITIONTEXTE et;
    memset(&et, 0, sizeof(EDITIONTEXTE));
    et.surfaceBlit = NULL;
    et.positionEdition.x = 30;
    et.positionBlit.h = 30;
    et.positionEdition.y = ecran->h - 20 - et.positionBlit.h;
    et.positionBlit.w = ecran->w - 60;
    et.couleurPP.r = 255; et.couleurPP.g = 255; et.couleurPP.b = 255;
    et.couleurAP.r = 0; et.couleurAP.g = 0; et.couleurAP.b = 0;
    et.couleurSelPP = et.couleurAP;
    et.couleurSelAP = et.couleurPP;
    et.police = TTF_OpenFont("times.ttf", 20);
    et.texte = NULL;
    strncpy(et.texteFixe, "Entrez votre équation : ", 200);
    et.typeBlit = TEXTE_BLIT_SHADED;
    et.flags = 0;
    et.focus = 1;

    EDITIONTEXTE et2 = et;
    et2.surfaceBlit = menu[1].img;
    et2.positionEdition.x = LARGEUR+10;
    et2.positionBlit.x = 27;
    et2.positionBlit.y = 100;
    et2.positionBlit.h = 19;
    et2.positionBlit.w = 50;
    et2.couleurPP = et.couleurAP;
    et2.couleurSelPP.r = 255;
    et2.couleurSelAP.b = 0;
    et2.police = TTF_OpenFont("forte.ttf", 15);
    et2.texteFixe[0] = '\0';
    et2.typeBlit = TEXTE_BLIT_BLENDED;
    et2.flags = TEXTE_FLAGS_SLMTNOMBRES | TEXTE_FLAGS_CENTRER;
    et2.focus = 0;

    TEXTE_CreerEditionEx(&et);
    TEXTE_CreerEditionEx(&et2);

    EDITIONTEXTE *tab = &et;


    surfaceCourbe = SDL_CreateRGBSurface(SDL_HWSURFACE, 700, 510, 32, 0, 0, 0, 0);
    SDL_SetColorKey(surfaceCourbe, SDL_SRCCOLORKEY, SDL_MapRGB(surfaceCourbe->format, 0, 0, 1));
    SDL_FillRect(surfaceCourbe, NULL, SDL_MapRGB(surfaceCourbe->format, 0, 0, 1));


    SDL_Flip(ecran);
    SDL_EnableKeyRepeat(500, 30);

    int continuer = 1, focus = et.focus, boutonEnclenche = -1, modeZoomCarre = 0;
    surfaceBoutons = InitBoutons();
    Uint8 *etatClavier;

    while (continuer)
    {
          SDL_WaitEvent(&event);

          etatClavier = SDL_GetKeyState(NULL);
          positionSecondCurseur = PositionnerSecondCurseur(ecran);
          TEXTE_RelayerEvent (&event, NULL, 0);
          if (menuActuel)
             RelayerEventBoutons(event, *menuActuel);

          switch (event.type)
          {
                 case SDL_QUIT:
                      continuer = 0;
                      break;
                 case SDL_KEYDOWN:
                      switch (event.key.keysym.sym)
                      {
                             case SDLK_ESCAPE:
                                  continuer = 0;
                                  break;
                             default:
                                  break;
                      }
                      break;
                 case SDL_MOUSEBUTTONDOWN:
                      if (modeZoomCarre == 1 && event.button.button == SDL_BUTTON_LEFT)
                      {
                          pointZoomCarre1.x = positionSecondCurseur.x;
                          pointZoomCarre1.y = positionSecondCurseur.y;
                          modeZoomCarre = 2;
                      }
                      else if (modeZoomCarre == 1)
                      {
                           modeZoomCarre = 0;
                           SDL_SetCursor(curseurNormal);
                           TEXTE_ModifierCurseurDefaut(curseurNormal);
                      }
                      break;
                 case SDL_MOUSEMOTION:
                 {
                      if (modeZoomCarre == 2)
                      {
                          pointZoomCarre2.y = positionSecondCurseur.y;
                          pointZoomCarre2.x = positionSecondCurseur.x;
                          SDL_Color violet = {255,0,255};
                          SDL_FillRect(surfaceAdditionelle, NULL, SDL_MapRGB(surfaceAdditionelle->format, 0, 0, 0));
                          DessinerRectangle2(surfaceAdditionelle, pointZoomCarre1, pointZoomCarre2, violet);
                      }
                      break;
                 }
                 case SDL_MOUSEBUTTONUP:
                      if (modeZoomCarre == 2)
                      {
                          SDL_FillRect(surfaceAdditionelle, NULL, SDL_MapRGB(surfaceAdditionelle->format, 0, 0, 0));
                          if (pointZoomCarre2.y != pointZoomCarre1.y && pointZoomCarre2.x != pointZoomCarre1.x)
                          {
                              OptionsTrace ot = optionsTrace;
                              double X, Y;

                              PixelVersCoord(pointZoomCarre1, ot, surfaceCourbe->w, surfaceCourbe->h, &X, &Y);
                              optionsTrace.xmin = X;
                              optionsTrace.ymax = Y;
                              PixelVersCoord(pointZoomCarre2, ot, surfaceCourbe->w, surfaceCourbe->h, &X, &Y);
                              optionsTrace.xmax = X;
                              optionsTrace.ymin = Y;
                          }

                          if (optionsTrace.xmax < optionsTrace.xmin)
                          {
                              int tmp = optionsTrace.xmax;
                              optionsTrace.xmax = optionsTrace.xmin;
                              optionsTrace.xmin = tmp;
                          }
                          if (optionsTrace.ymax < optionsTrace.ymin)
                          {
                              int tmp = optionsTrace.ymax;
                              optionsTrace.ymax = optionsTrace.ymin;
                              optionsTrace.ymin = tmp;
                          }

                          if (etatClavier[SDLK_LSHIFT])
                          {
                              optionsTrace.xmax = round(optionsTrace.xmax);
                              optionsTrace.ymax = round(optionsTrace.ymax);
                              optionsTrace.ymin = round(optionsTrace.ymin);
                              optionsTrace.xmin = round(optionsTrace.xmin);
                          }

                          focus = 1;
                          et.focus = 0;
                          modeZoomCarre = 0;
                          SDL_SetCursor(curseurNormal);
                          TEXTE_ModifierCurseurDefaut(curseurNormal);
                      }
                      break;
          }

          if (focus && !et.focus)
          {
             SDL_FillRect(surfaceCourbe, NULL, SDL_MapRGB(surfaceCourbe->format, 0, 0, 1));
             TracerCourbe(et.texte, optionsTrace.variable, optionsTrace, optionsParam[optionsTrace.typeCourbe], surfaceCourbe);

             OptionsTrace ot = optionsTrace;
             OptionsParam op;
             for (i = 0 ; i < MAX_FONCTIONS && tabFonctions[i] ; i++)
             {
                 if (tabFonctions[i]->active)
                 {
                    op = optionsParam[tabFonctions[i]->type];

                    ot.couleurTrace = tabFonctions[i]->couleur;
                    ot.typeCourbe = tabFonctions[i]->type;
                    op.borne1 = tabFonctions[i]->borne1;
                    op.borne2 = tabFonctions[i]->borne2;
                    if (op.borne1 != op.borne2)
                       op.bornesAuto = 0;
                    else op.bornesAuto = 1;
                    TracerCourbe(tabFonctions[i]->expression, tabFonctions[i]->variable, ot, op, surfaceCourbe);
                 }
              }
          }
          focus = et.focus;
          TEXTE_ToutAttendre(NULL, 0);

          if (menuActuel)
             boutonEnclenche = AttendreBoutons();

          switch (boutonEnclenche)
          {
                 case IDMENUZOOM:
                      ChangerMenu(&(menu[1]));
                      break;
                 case IDMENUGRAPH:
                      ChangerMenu(&(menu[2]));
                      break;
                 case IDZOOMPLUS:
                 case IDZOOMMOINS:
                 {
                     double fact = strtod(et2.texte, NULL);
                     if (fact)
                     {
                         if (boutonEnclenche == IDZOOMPLUS)
                            fact = 1/fact;

                         double milieu = (optionsTrace.xmax+optionsTrace.xmin) / 2.0;
                         optionsTrace.xmin = milieu-fact*(milieu-optionsTrace.xmin);
                         optionsTrace.xmax = milieu+fact*(optionsTrace.xmax-milieu);
                         milieu = (optionsTrace.ymax+optionsTrace.ymin) / 2.0;
                         optionsTrace.ymin = milieu-fact*(milieu-optionsTrace.ymin);
                         optionsTrace.ymax = milieu+fact*(optionsTrace.ymax-milieu);

                         focus = 1;
                         DeclencherEvent();
                     }
                     break;
                 }
                 case IDZOOMNORM:
                 {
                      if (derniereEditionRepere)
                      {
                         double echelleY = 1.0*surfaceCourbe->h / (optionsTrace.ymax - optionsTrace.ymin),
                                xmax = surfaceCourbe->w/echelleY + optionsTrace.xmin,
                                diff = optionsTrace.xmax-xmax;

                         optionsTrace.xmin += diff/2;
                         optionsTrace.xmax = xmax + diff/2;
                      }
                      else
                      {
                          double echelleX = 1.0*surfaceCourbe->w / (optionsTrace.xmax - optionsTrace.xmin),
                                 ymax = surfaceCourbe->h/echelleX + optionsTrace.ymin,
                                 diff = optionsTrace.ymax-ymax;

                          optionsTrace.ymin += diff/2;
                          optionsTrace.ymax = ymax + diff/2;
                      }

                      focus = 1;
                      DeclencherEvent();
                      break;
                 }
                 case IDZOOMDEF:
                 {
                      optionsTrace.xmin = -6.3;
                      optionsTrace.xmax = 6.3;
                      optionsTrace.xech = 1;
                      optionsTrace.ymin = -3.1;
                      optionsTrace.ymax = 3.1;
                      optionsTrace.yech = 1;

                      focus = 1;
                      DeclencherEvent();
                      break;
                 }
                 case IDZOOMCARRE:
                      modeZoomCarre = 1;
                      SDL_SetCursor(curseurCroix);
                      TEXTE_ModifierCurseurDefaut(curseurCroix);
                      break;
                 case IDREGLAGES:
                      if (!fenetreOptions_Wnd)
                         CreateThread(NULL, 0, ThreadFenetreOptions, NULL, 0, NULL);
                      else SetWindowPos(fenetreOptions_Wnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
                      break;
          }


          if (menuActuel && menuActuel->pos.x >= LARGEUR)
          {
              ancienMenu = menuActuel;
              menuActuel = NULL;
          }

          if (menuActuel == &(menu[1]))
            et2.positionEdition = menuActuel->pos;

          if (menuActuel && TestBouton(event, surfaceBoutons, &boutonDisparition))
             ChangerMenu(NULL);
          else if (!menuActuel)
          {
              SDL_FillRect(surfaceBoutons, 0, SDL_MapRGB(surfaceBoutons->format, 0, 0, 0));
              if (TestBouton(event, surfaceBoutons, &boutonApparition))
                 ChangerMenu(&(menu[0]));
          }

          SDL_BlitSurface(imageDeFond, NULL, ecran, &positionFond);
          AjouterCoordonnees(ecran, courrierNew);
          SDL_BlitSurface(surfaceCourbe, NULL, ecran, &positionFond);
          if (etatClavier[SDLK_LSHIFT])
          {
              SDL_Rect rect = {positionSecondCurseur.x-secondCurseur->w/2, positionSecondCurseur.y-secondCurseur->h/2};
              SDL_BlitSurface(secondCurseur, NULL, ecran, &rect);
          }
          SDL_BlitSurface(surfaceAdditionelle, NULL, ecran, &positionFond);
          if (ancienMenu)
             SDL_BlitSurface(ancienMenu->img, NULL, ecran, &(ancienMenu->pos));
          if (menuActuel)
          {
             SDL_BlitSurface(menuActuel->img, NULL, ecran, &(menuActuel->pos));
             boutonDisparition.dim.x = menuActuel->pos.x - boutonDisparition.dim.w - 3;
             SDL_BlitSurface(boutonDisparition.img, NULL, ecran, &(boutonDisparition.dim));
          }
          else
          {
              boutonApparition.dim.x = LARGEUR - boutonApparition.dim.w - 3;
              SDL_BlitSurface(boutonApparition.img, NULL, ecran, &(boutonApparition.dim));
          }
          SDL_BlitSurface(surfaceBoutons, NULL, ecran, &positionFond);
          TEXTE_ToutBlitter (&tab, 1, ecran);
          SDL_Flip(ecran);

    }


    TEXTE_ToutEffacer(NULL, 0, 1);

    SDL_FreeSurface(imageDeFond);
    SDL_FreeSurface(ecran);
    SDL_Quit();
    TTF_CloseFont(et.police);
    TTF_CloseFont(et2.police);
    TTF_Quit();
    return EXIT_SUCCESS;
}


LRESULT CALLBACK Options_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                fenetreOptions_Wnd = hwnd;
                CentrerFenetre(hwnd, NULL);
                RECT taille;
                GetClientRect(hwnd, &taille);
                int largeur = 150;

                HWND listeWnd = CreateWindowEx(
                     WS_EX_CLIENTEDGE,
                     WC_LISTVIEW,
                     "",
                     WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_NOCOLUMNHEADER | LVS_SINGLESEL,
                     15,
                     15,
                     largeur,
                     taille.bottom-30,
                     hwnd,
                     (HMENU) IDLV_LISTEOPTIONS,
                     GetModuleHandle(NULL),
                     NULL
                );
                ListView_SetExtendedListViewStyle(listeWnd, LVS_EX_FULLROWSELECT);

                LVCOLUMN lvcol;
                lvcol.mask = LVCF_WIDTH;
                lvcol.cx = largeur-6;
                ListView_InsertColumn(listeWnd, 0, &lvcol);

                LVITEM lvi;
                lvi.mask = LVIF_TEXT;
                lvi.iItem = 0;
                lvi.iSubItem = 0;
                lvi.pszText = buffer;
                strcpy(buffer, "Tracé");
                ListView_InsertItem(listeWnd, &lvi);

                strcpy(buffer, "Paramétrage");
                lvi.iItem++;
                ListView_InsertItem(listeWnd, &lvi);

                strcpy(buffer, "Algorithmes");
                lvi.iItem++;
                ListView_InsertItem(listeWnd, &lvi);

                strcpy(buffer, "Fonctions");
                lvi.iItem++;
                ListView_InsertItem(listeWnd, &lvi);

                strcpy(buffer, "Variables");
                lvi.iItem++;
                ListView_InsertItem(listeWnd, &lvi);

                strcpy(buffer, "Rappels utiles");
                lvi.iItem++;
                ListView_InsertItem(listeWnd, &lvi);

                strcpy(buffer, "Affichage");
                lvi.iItem++;
                ListView_InsertItem(listeWnd, &lvi);

                strcpy(buffer, "Configurations");
                lvi.iItem++;
                ListView_InsertItem(listeWnd, &lvi);

                optionsTrace_Wnd = CreateDialog(GetModuleHandle(NULL), "OptionsTrace", hwnd, (DLGPROC)OptionsTrace_proc);
                SetWindowPos(optionsTrace_Wnd, NULL, largeur+30, 15, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);

                optionsParam_Wnd = CreateDialog(GetModuleHandle(NULL), "OptionsParam", hwnd, (DLGPROC)OptionsParam_proc);
                SetWindowPos(optionsParam_Wnd, NULL, largeur+30, 15, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

                optionsAlgo_Wnd = CreateDialog(GetModuleHandle(NULL), "OptionsAlgo", hwnd, (DLGPROC)OptionsAlgo_proc);
                SetWindowPos(optionsAlgo_Wnd, NULL, largeur+30, 15, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

                optionsFonctions_Wnd = CreateDialog(GetModuleHandle(NULL), "OptionsFonctions", hwnd, (DLGPROC)OptionsFonctions_proc);
                SetWindowPos(optionsFonctions_Wnd, NULL, largeur+30, 15, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

                optionsVariables_Wnd = CreateDialog(GetModuleHandle(NULL), "OptionsVariables", hwnd, (DLGPROC)OptionsVariables_proc);
                SetWindowPos(optionsVariables_Wnd, NULL, largeur+30, 15, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

                rappels_Wnd = CreateDialog(GetModuleHandle(NULL), "Rappels", hwnd, (DLGPROC)Rappels_proc);
                SetWindowPos(rappels_Wnd, NULL, largeur+30, 15, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                return FALSE;
           }
           case WM_NOTIFY:
           {
                NMHDR *nmhdr = (NMHDR*)lParam;
                switch (nmhdr->idFrom)
                {
                       case IDLV_LISTEOPTIONS:
                       {
                            if (nmhdr->code != NM_CLICK)
                               return FALSE;

                            NMITEMACTIVATE *nmia = (NMITEMACTIVATE*)lParam;
                            if (nmia->iItem < 0 || derniereSelectionListeOptions == nmia->iItem)
                               return FALSE;


                            ShowWindow(optionsTrace_Wnd, SW_HIDE);
                            ShowWindow(optionsParam_Wnd, SW_HIDE);
                            ShowWindow(optionsAlgo_Wnd, SW_HIDE);
                            ShowWindow(optionsFonctions_Wnd, SW_HIDE);
                            ShowWindow(optionsVariables_Wnd, SW_HIDE);
                            ShowWindow(rappels_Wnd, SW_HIDE);

                            switch (nmia->iItem)
                            {
                                   case 0:
                                        ShowWindow(optionsTrace_Wnd, SW_SHOW);
                                        break;
                                   case 1:
                                        ShowWindow(optionsParam_Wnd, SW_SHOW);
                                        break;
                                   case 2:
                                        ShowWindow(optionsAlgo_Wnd, SW_SHOW);
                                        break;
                                   case 3:
                                        ShowWindow(optionsFonctions_Wnd, SW_SHOW);
                                        break;
                                   case 4:
                                        ShowWindow(optionsVariables_Wnd, SW_SHOW);
                                        break;
                                   case 5:
                                        ShowWindow(rappels_Wnd, SW_SHOW);
                                        break;
                            }

                            derniereSelectionListeOptions = nmia->iItem;
                            return FALSE;
                       }
                }

                return FALSE;
           }
           case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                       case IDP_OK:
                       case IDP_APPLIQUER:
                       {

                            //première fenêtre
                            OptionsTrace ot;

                            GetDlgItemText(optionsTrace_Wnd, IDE_NOMBREPOINTS, buffer, MAX_CHAINE);
                            ot.nombrePoints = strtol(buffer, NULL, 10);
                            if (ot.nombrePoints <= 10)
                            {
                               MessageBox(hwnd, "Nombre de points trop faible !", "Attention", MB_OK | MB_ICONWARNING);
                               return FALSE;
                            }
                            else if (ot.nombrePoints >= 10000 && MessageBox(hwnd, "Le nombre de points est très grand. Ceci peut ralentir les calculs.\nContinuer ?", "Attention", MB_YESNO | MB_ICONWARNING) == IDNO)
                                 return FALSE;

                            if (IsDlgButtonChecked(optionsTrace_Wnd, IDC_RELIER))
                               ot.relierPoints = 1;
                            if (IsDlgButtonChecked(optionsTrace_Wnd, IDC_TESTDISCONT))
                               ot.testDiscontinuite = 1;

                            GetDlgItemText(optionsTrace_Wnd, IDE_XMIN, buffer, MAX_CHAINE);
                            ot.xmin = strtod(buffer, NULL);
                            GetDlgItemText(optionsTrace_Wnd, IDE_XMAX, buffer, MAX_CHAINE);
                            ot.xmax = strtod(buffer, NULL);
                            GetDlgItemText(optionsTrace_Wnd, IDE_YMAX, buffer, MAX_CHAINE);
                            ot.ymax = strtod(buffer, NULL);
                            GetDlgItemText(optionsTrace_Wnd, IDE_YMIN, buffer, MAX_CHAINE);
                            ot.ymin = strtod(buffer, NULL);
                            GetDlgItemText(optionsTrace_Wnd, IDE_XECH, buffer, MAX_CHAINE);
                            ot.xech = strtod(buffer, NULL);
                            GetDlgItemText(optionsTrace_Wnd, IDE_YECH, buffer, MAX_CHAINE);
                            ot.yech = strtod(buffer, NULL);
                            if (ot.xmin == ot.xmax || ot.ymin == ot.ymax || !ot.xech || !ot.yech)
                            {
                               MessageBox(hwnd, "Coordonnées de repère incorrectes !", "Attention", MB_OK | MB_ICONWARNING);
                               return FALSE;
                            }

                            GetDlgItemText(optionsTrace_Wnd, IDCB_VARIABLE, buffer, 2);
                            if (!TestLettre(buffer[0]))
                            {
                               MessageBox(hwnd, "Variable d'équation invalide !", "Attention", MB_OK | MB_ICONWARNING);
                               return FALSE;
                            }
                            else ot.variable = buffer[0];

                            GetDlgItemText(optionsTrace_Wnd, IDE_XORIGINE, buffer, MAX_CHAINE);
                            ot.xorigine = strtod(buffer, NULL);
                            GetDlgItemText(optionsTrace_Wnd, IDE_YORIGINE, buffer, MAX_CHAINE);
                            ot.yorigine = strtod(buffer, NULL);
                            GetDlgItemText(optionsTrace_Wnd, IDE_ROTATIONREPERE, buffer, MAX_CHAINE);
                            ot.rotation = strtod(buffer, NULL);

                            ot.couleurTrace = couleurTrace;
                            ot.couleurRepere = couleurRepere;

                            ot.typeCourbe = ComboBox_GetCurSel(GetDlgItem(optionsTrace_Wnd, IDCB_TYPECOURBE));


                            //deuxième fenêtre
                            int curSel = ComboBox_GetCurSel(GetDlgItem(optionsParam_Wnd, IDCB_TYPECOURBE));
                            if (!SauverOptionsParam(&(optionsParamTemp[curSel+1])))
                               return FALSE;


                            //troisième fenêtre
                            OptionsAlgo oa;

                            if (IsDlgButtonChecked(optionsAlgo_Wnd, IDR_AIRES))
                               oa.calculIntg = AIRES;
                            else if (IsDlgButtonChecked(optionsAlgo_Wnd, IDR_GAUSS))
                               oa.calculIntg = GAUSS;
                            else if (IsDlgButtonChecked(optionsAlgo_Wnd, IDR_PROBA))
                               oa.calculIntg = PROBA;
                            else if (IsDlgButtonChecked(optionsAlgo_Wnd, IDR_PRIMITIVE))
                               oa.calculIntg = PRIMITIVE;
                            else oa.calculIntg = RIEMANN;

                            if (IsDlgButtonChecked(optionsAlgo_Wnd, IDR_EULER))
                               oa.calculPrimitive = EULER;
                            else oa.calculPrimitive = INTEGRATION;

                            GetDlgItemText(optionsAlgo_Wnd, IDE_POINTSTESTS1, buffer, MAX_CHAINE);
                            oa.nombrePointsI = strtol(buffer, NULL, 10);
                            GetDlgItemText(optionsAlgo_Wnd, IDE_POINTSTESTS2, buffer, MAX_CHAINE);
                            oa.nombrePointsP = strtol(buffer, NULL, 10);
                            if ((oa.calculIntg != GAUSS && oa.calculIntg != PRIMITIVE && oa.nombrePointsI <= 10) || (oa.calculPrimitive == EULER && oa.nombrePointsP <= 10))
                            {
                               MessageBox(hwnd, "Nombre de points tests trop faible !", "Attention", MB_OK | MB_ICONWARNING);
                               return FALSE;
                            }
                            else if (((oa.calculIntg != GAUSS && oa.calculIntg != PRIMITIVE && oa.nombrePointsI >= 10000) || (oa.calculPrimitive == EULER && oa.nombrePointsP >= 10000))
                                    && MessageBox(hwnd, "Le nombre de points tests est très grand. Ceci peut ralentir les calculs.\nContinuer ?", "Attention", MB_YESNO | MB_ICONWARNING) == IDNO)
                                 return FALSE;


                            //quatrième fenêtre
                            /* rien */

                            //cinquième fenêtre
                            /* rien */


                            //sauvegarde définitive
                            optionsTrace = ot;
                            int i;
                            for (i = 0 ; i < 4 ; i++)
                                optionsParam[i] = optionsParamTemp[i];
                            optionsAlgo = oa;
                            MettreAJourTabFonctions(listeFonctions_Wnd, NULL, 0);
                            MettreAJourTabVariables(listeVariables_Wnd, NULL, 0);

                            if (LOWORD(wParam) == IDP_OK)
                            {
                               fenetreOptions_Wnd = NULL;
                               EndDialog(hwnd, 1);
                            }
                            return FALSE;
                       }
                       case IDP_ANNULER:
                            fenetreOptions_Wnd = NULL;
                            EndDialog(hwnd, 0);
                            return FALSE;
                }

                return FALSE;
           case WM_CLOSE:
                fenetreOptions_Wnd = NULL;
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}

LRESULT CALLBACK OptionsTrace_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                SendMessage(GetDlgItem(hwnd, IDGB_OPTIONSTRACE), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);
                SendMessage(GetDlgItem(hwnd, IDGB_REGLAGESREPERE), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);
                SendMessage(GetDlgItem(hwnd, IDGB_CHANGEMENTREPERE), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);
                SendMessage(GetDlgItem(hwnd, IDGB_OPTIONSCOURBE), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);

                sprintf(buffer, "%d", optionsTrace.nombrePoints);
                SetDlgItemText(hwnd, IDE_NOMBREPOINTS, buffer);

                if (optionsTrace.relierPoints)
                   CheckDlgButton(hwnd, IDC_RELIER, BST_CHECKED);
                else EnableWindow(GetDlgItem(hwnd, IDC_TESTDISCONT), FALSE);

                if (optionsTrace.testDiscontinuite)
                   CheckDlgButton(hwnd, IDC_TESTDISCONT, BST_CHECKED);

                sprintf(buffer, "%.2f", optionsTrace.ymin);
                SetDlgItemText(hwnd, IDE_YMIN, buffer);
                sprintf(buffer, "%.2f", optionsTrace.ymax);
                SetDlgItemText(hwnd, IDE_YMAX, buffer);
                sprintf(buffer, "%.2f", optionsTrace.xmin);
                SetDlgItemText(hwnd, IDE_XMIN, buffer);
                sprintf(buffer, "%.2f", optionsTrace.xmax);
                SetDlgItemText(hwnd, IDE_XMAX, buffer);
                sprintf(buffer, "%.2f", optionsTrace.xech);
                SetDlgItemText(hwnd, IDE_XECH, buffer);
                sprintf(buffer, "%.2f", optionsTrace.yech);
                SetDlgItemText(hwnd, IDE_YECH, buffer);
                sprintf(buffer, "%.2f", optionsTrace.xorigine);
                SetDlgItemText(hwnd, IDE_XORIGINE, buffer);
                sprintf(buffer, "%.2f", optionsTrace.yorigine);
                SetDlgItemText(hwnd, IDE_YORIGINE, buffer);
                sprintf(buffer, "%.2f", optionsTrace.rotation);
                SetDlgItemText(hwnd, IDE_ROTATIONREPERE, buffer);

                couleurTrace = optionsTrace.couleurTrace;
                carreCouleurTrace_Wnd = CreateDialog(GetModuleHandle(NULL), "Couleur", hwnd, (DLGPROC)Couleur_proc);
                SetWindowPos(carreCouleurTrace_Wnd, NULL, 113, 115, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);

                sprintf(buffer, "0x%06X", (unsigned int)optionsTrace.couleurTrace);
                SetDlgItemText(hwnd, IDT_COULEURTRACE, buffer);

                couleurRepere = optionsTrace.couleurRepere;
                carreCouleurRepere_Wnd = CreateDialog(GetModuleHandle(NULL), "Couleur", hwnd, (DLGPROC)Couleur_proc);
                SetWindowPos(carreCouleurRepere_Wnd, NULL, 121, 351, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);

                sprintf(buffer, "0x%06X", (unsigned int)optionsTrace.couleurRepere);
                SetDlgItemText(hwnd, IDT_COULEURREPERE, buffer);

                HWND comboBox = GetDlgItem(hwnd, IDCB_TYPECOURBE);
                ComboBox_AddString(comboBox, "Cartésienne");
                ComboBox_AddString(comboBox, "Paramétrée");
                ComboBox_AddString(comboBox, "Polaire");
                ComboBox_AddString(comboBox, "Paramétrée polaire");
                ComboBox_SetCurSel(comboBox, 0);

                comboBox = GetDlgItem(hwnd, IDCB_VARIABLE);
                ComboBox_AddString(comboBox, "x");
                ComboBox_AddString(comboBox, "t");
                ComboBox_AddString(comboBox, "o");
                ComboBox_SetCurSel(comboBox, 0);
                SendMessage(comboBox, CB_LIMITTEXT, 1, 0);

                return FALSE;
           }
           case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                       case IDC_RELIER:
                            if (IsDlgButtonChecked(hwnd, IDC_RELIER))
                               EnableWindow(GetDlgItem(hwnd, IDC_TESTDISCONT), TRUE);
                            else EnableWindow(GetDlgItem(hwnd, IDC_TESTDISCONT), FALSE);

                            return FALSE;
                       case IDP_REPEREDEFAUT:
                            strcpy(buffer, "-6.30");
                            SetDlgItemText(hwnd, IDE_XMIN, buffer);
                            strcpy(buffer, "6.30");
                            SetDlgItemText(hwnd, IDE_XMAX, buffer);
                            strcpy(buffer, "-3.10");
                            SetDlgItemText(hwnd, IDE_YMIN, buffer);
                            strcpy(buffer, "3.10");
                            SetDlgItemText(hwnd, IDE_YMAX, buffer);
                            strcpy(buffer, "1.00");
                            SetDlgItemText(hwnd, IDE_XECH, buffer);
                            SetDlgItemText(hwnd, IDE_YECH, buffer);

                            return FALSE;
                       case IDP_COULEURTRACE:
                       case IDP_COULEURREPERE:
                       {
                            CHOOSECOLOR cl;
                            COLORREF couleur;
                            memset(&cl, 0, sizeof(CHOOSECOLOR));

                            cl.lStructSize = sizeof(CHOOSECOLOR);
                            cl.hwndOwner = hwnd;
                            cl.lpCustColors = &couleur;
                            cl.Flags = CC_FULLOPEN;
                            if (ChooseColor(&cl))
                            {
                                sprintf(buffer, "0x%06X", (unsigned int)cl.rgbResult);

                                if (LOWORD(wParam) == IDP_COULEURTRACE)
                                {
                                   couleurTrace = cl.rgbResult;
                                   SetDlgItemText(hwnd, IDT_COULEURTRACE, buffer);
                                   InvalidateRect(carreCouleurTrace_Wnd, NULL, FALSE);
                                   UpdateWindow(carreCouleurTrace_Wnd);
                                }
                                else if (LOWORD(wParam) == IDP_COULEURREPERE)
                                {
                                   couleurRepere = cl.rgbResult;
                                   SetDlgItemText(hwnd, IDT_COULEURREPERE, buffer);
                                   InvalidateRect(carreCouleurRepere_Wnd, NULL, FALSE);
                                   UpdateWindow(carreCouleurRepere_Wnd);
                                }
                            }

                            return FALSE;
                       }
                       case IDP_REPERENORME:
                       {
                            if (!surfaceCourbe)
                               return FALSE;

                            if (derniereEditionRepere)
                            {
                               GetDlgItemText(hwnd, IDE_YMIN, buffer, MAX_CHAINE);
                               double ymin = strtod(buffer, NULL);
                               GetDlgItemText(hwnd, IDE_YMAX, buffer, MAX_CHAINE);
                               double ymax = strtod(buffer, NULL),
                                      echelleY = 1.0*surfaceCourbe->h / (ymax - ymin);

                               GetDlgItemText(hwnd, IDE_XMIN, buffer, MAX_CHAINE);
                               double xmin = strtod(buffer, NULL),
                                      xmax = surfaceCourbe->w/echelleY + xmin;
                               GetDlgItemText(hwnd, IDE_XMAX, buffer, MAX_CHAINE);
                               double diff = strtod(buffer, NULL)-xmax;

                               xmin += diff/2;
                               xmax += diff/2;

                               sprintf(buffer, "%.2f", xmin);
                               SetDlgItemText(hwnd, IDE_XMIN, buffer);
                               sprintf(buffer, "%.2f", xmax);
                               SetDlgItemText(hwnd, IDE_XMAX, buffer);
                            }
                            else
                            {
                                GetDlgItemText(hwnd, IDE_XMIN, buffer, MAX_CHAINE);
                                double xmin = strtod(buffer, NULL);
                                GetDlgItemText(hwnd, IDE_XMAX, buffer, MAX_CHAINE);
                                double xmax = strtod(buffer, NULL),
                                       echelleX = 1.0*surfaceCourbe->w / (xmax - xmin);

                                GetDlgItemText(hwnd, IDE_YMIN, buffer, MAX_CHAINE);
                                double ymin = strtod(buffer, NULL),
                                       ymax = surfaceCourbe->h/echelleX + ymin;
                                GetDlgItemText(hwnd, IDE_YMAX, buffer, MAX_CHAINE);
                                double diff = strtod(buffer, NULL)-ymax;

                                ymin += diff/2;
                                ymax += diff/2;

                                sprintf(buffer, "%.2f", ymin);
                                SetDlgItemText(hwnd, IDE_YMIN, buffer);
                                sprintf(buffer, "%.2f", ymax);
                                SetDlgItemText(hwnd, IDE_YMAX, buffer);
                            }

                            return FALSE;
                       }
                       case IDCB_TYPECOURBE:
                       {
                            if (HIWORD(wParam) != CBN_SELCHANGE)
                               return FALSE;

                            int sel = ComboBox_GetCurSel(GetDlgItem(hwnd, IDCB_TYPECOURBE));
                            HWND comboBox = GetDlgItem(hwnd, IDCB_VARIABLE);
                            if (sel != 3)
                               ComboBox_SetCurSel(comboBox, sel);
                            else ComboBox_SetCurSel(comboBox, 1);

                            SetFocus(GetDlgItem(hwnd, IDP_OK));
                            return FALSE;
                       }
                       case IDE_XMAX:
                       case IDE_XMIN:
                            if (HIWORD(wParam) == EN_CHANGE)
                               derniereEditionRepere = 0;
                            return FALSE;
                       case IDE_YMAX:
                       case IDE_YMIN:
                            if (HIWORD(wParam) == EN_CHANGE)
                               derniereEditionRepere = 1;
                            return FALSE;
                }
                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}

LRESULT CALLBACK OptionsParam_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
           case WM_INITDIALOG:
           {
                SendMessage(GetDlgItem(hwnd, IDGB_OPTIONSPARAM), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);

                HWND comboBox = GetDlgItem(hwnd, IDCB_TYPECOURBE);
                ComboBox_AddString(comboBox, "Paramétrées");
                ComboBox_AddString(comboBox, "Polaires");
                ComboBox_AddString(comboBox, "Paramétrées polaires");
                ComboBox_SetCurSel(comboBox, 0);
                selectionPrecedente = 0;

                optionsParam_Wnd = hwnd;
                ReglerFenetreOptionsParam(optionsParamTemp);
                return FALSE;
           }
           case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                       case IDCB_TYPECOURBE:
                       {
                            if (HIWORD(wParam) != CBN_SELCHANGE)
                               return FALSE;

                            HWND comboBox = GetDlgItem(hwnd, IDCB_TYPECOURBE);

                            if (!SauverOptionsParam(&(optionsParamTemp[selectionPrecedente+1])))
                               ComboBox_SetCurSel(comboBox, selectionPrecedente);
                            else
                            {
                                selectionPrecedente = ComboBox_GetCurSel(comboBox);
                                ReglerFenetreOptionsParam(optionsParamTemp);
                            }

                            return FALSE;
                       }
                       return FALSE;
                       case IDR_MANUVARIATION:
                            EnableWindow(GetDlgItem(hwnd, IDE_BORNE1), TRUE);
                            EnableWindow(GetDlgItem(hwnd, IDE_BORNE2), TRUE);
                            return FALSE;
                       case IDR_AUTOVARIATION:
                            EnableWindow(GetDlgItem(hwnd, IDE_BORNE1), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDE_BORNE2), FALSE);
                            return FALSE;
                       case IDC_PASFIXE:
                            EnableWindow(GetDlgItem(hwnd, IDE_PAS), IsDlgButtonChecked(hwnd, IDC_PASFIXE));
                            return FALSE;
                       case IDC_CONTINUERTRACE:
                            EnableWindow(GetDlgItem(hwnd, IDE_POINTSSUP), IsDlgButtonChecked(hwnd, IDC_CONTINUERTRACE));
                            return FALSE;
                       case IDC_ARRETERTRACE:
                            EnableWindow(GetDlgItem(hwnd, IDE_POINTSDEHORS), IsDlgButtonChecked(hwnd, IDC_ARRETERTRACE));
                            EnableWindow(GetDlgItem(hwnd, IDC_POINTSDEHORSCONSECUTIFS), IsDlgButtonChecked(hwnd, IDC_ARRETERTRACE));
                            return FALSE;

                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}

LRESULT CALLBACK OptionsAlgo_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "\0";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                SendMessage(GetDlgItem(hwnd, IDGB_CALCULINTG), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);
                SendMessage(GetDlgItem(hwnd, IDGB_PRIMITIVES), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);

                switch (optionsAlgo.calculIntg)
                {
                       case AIRES:
                            CheckDlgButton(hwnd, IDR_AIRES, BST_CHECKED);
                            break;
                       case GAUSS:
                            CheckDlgButton(hwnd, IDR_GAUSS, BST_CHECKED);
                            EnableWindow(GetDlgItem(hwnd, IDE_POINTSTESTS1), FALSE);
                            break;
                       case PROBA:
                            CheckDlgButton(hwnd, IDR_PROBA, BST_CHECKED);
                            break;
                       case PRIMITIVE:
                            CheckDlgButton(hwnd, IDR_PRIMITIVE, BST_CHECKED);
                            EnableWindow(GetDlgItem(hwnd, IDE_POINTSTESTS1), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDR_INTG), FALSE);
                            break;
                       default:
                            CheckDlgButton(hwnd, IDR_RIEMANN, BST_CHECKED);
                            break;
                }

                if (optionsAlgo.calculPrimitive == EULER)
                   CheckDlgButton(hwnd, IDR_EULER, BST_CHECKED);
                else
                {
                    CheckDlgButton(hwnd, IDR_INTG, BST_CHECKED);
                    EnableWindow(GetDlgItem(hwnd, IDR_PRIMITIVE), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDE_POINTSTESTS2), FALSE);
                }

                sprintf(buffer, "%d", optionsAlgo.nombrePointsI);
                SetDlgItemText(hwnd, IDE_POINTSTESTS1, buffer);
                sprintf(buffer, "%d", optionsAlgo.nombrePointsP);
                SetDlgItemText(hwnd, IDE_POINTSTESTS2, buffer);

                return FALSE;
           }
           case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                       case IDR_PRIMITIVE:
                            EnableWindow(GetDlgItem(hwnd, IDE_POINTSTESTS1), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDR_INTG), FALSE);
                            break;
                       case IDR_GAUSS:
                            EnableWindow(GetDlgItem(hwnd, IDE_POINTSTESTS1), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDR_INTG), TRUE);
                            break;
                       case IDR_RIEMANN:
                       case IDR_AIRES:
                       case IDR_PROBA:
                            EnableWindow(GetDlgItem(hwnd, IDE_POINTSTESTS1), TRUE);
                            EnableWindow(GetDlgItem(hwnd, IDR_INTG), TRUE);
                            break;
                       case IDR_EULER:
                            EnableWindow(GetDlgItem(hwnd, IDE_POINTSTESTS2), TRUE);
                            EnableWindow(GetDlgItem(hwnd, IDR_PRIMITIVE), TRUE);
                            break;
                       case IDR_INTG:
                            EnableWindow(GetDlgItem(hwnd, IDE_POINTSTESTS2), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDR_PRIMITIVE), FALSE);
                            break;
                }

                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}


LRESULT CALLBACK OptionsFonctions_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "X";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                RECT taille;
                GetClientRect(hwnd, &taille);
                SendMessage(GetDlgItem(hwnd, IDGB_FONCTIONS), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);

                HWND listeWnd = CreateWindowEx(
                     WS_EX_CLIENTEDGE,
                     WC_LISTVIEW,
                     "",
                     WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
                     15,
                     30,
                     taille.right-30,
                     taille.bottom-85,
                     hwnd,
                     (HMENU) IDLV_LISTEFONCTIONS,
                     GetModuleHandle(NULL),
                     NULL
                );
                ListView_SetExtendedListViewStyle(listeWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);

                LVCOLUMN lvcol;
                lvcol.mask = LVCF_WIDTH | LVCF_TEXT;
                lvcol.pszText = buffer;
                lvcol.cx = 25;
                ListView_InsertColumn(listeWnd, 0, &lvcol);

                lvcol.cx = taille.right-300;
                strcpy(buffer, "Nom et Expression");
                ListView_InsertColumn(listeWnd, 1, &lvcol);

                lvcol.cx = 120;
                strcpy(buffer, "Domaine");
                ListView_InsertColumn(listeWnd, 2, &lvcol);

                lvcol.cx = 120;
                strcpy(buffer, "Type");
                ListView_InsertColumn(listeWnd, 3, &lvcol);

                lvcol.cx = 0;
                strcpy(buffer, "Couleur");
                ListView_InsertColumn(listeWnd, 4, &lvcol);

                listeFonctions_Wnd = listeWnd;

                EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);
                return FALSE;
           }
           case WM_NOTIFY:
           {
                NMHDR *nmhdr = (NMHDR*)lParam;
                switch (nmhdr->idFrom)
                {
                       case IDLV_LISTEFONCTIONS:
                       {
                            if (nmhdr->code == NM_CLICK)
                            {
                               NMITEMACTIVATE *nmia = (NMITEMACTIVATE*)lParam;
                               int etat = 1;

                               if (nmia->iItem < 0)
                                  etat = 0;

                               EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), etat);
                               EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), etat);
                            }

                            if (nmhdr->code == NM_DBLCLK)
                            {
                               NMITEMACTIVATE *nmia = (NMITEMACTIVATE*)lParam;
                               if (nmia->iItem >= 0)
                                  SendMessage(hwnd, WM_COMMAND, IDP_MODIFIER, 0);
                            }

                            if (nmhdr->code == LVN_KEYDOWN)
                            {
                               NMLVKEYDOWN *nmlvkd = (NMLVKEYDOWN*)lParam;
                               if (nmlvkd->wVKey == VK_RETURN)
                                  SendMessage(hwnd, WM_COMMAND, IDP_MODIFIER, 0);
                               else if (nmlvkd->wVKey == VK_DELETE)
                                  SendMessage(hwnd, WM_COMMAND, IDP_SUPPRIMER, 0);
                            }

                            if (nmhdr->code == NM_CUSTOMDRAW)
                            {
                               NMLVCUSTOMDRAW *NMCustomDraw = (NMLVCUSTOMDRAW*) lParam;
                               if ((NMCustomDraw->nmcd.dwDrawStage & CDDS_ITEMPREPAINT) != CDDS_ITEMPREPAINT)
                                  return CDRF_NOTIFYSUBITEMDRAW;

                               HWND listeWnd = GetDlgItem(hwnd, IDLV_LISTEFONCTIONS);
                               int i,
                               nombreItems = ListView_GetItemCount(listeWnd);
                               HBRUSH brush = CreateSolidBrush(0);
                               RECT rect;

                               for (i = 0 ; i < nombreItems ; i++)
                               {
                                   ListView_GetItemText(listeWnd, i, 4, buffer, MAX_CHAINE);
                                   brush = CreateSolidBrush(strtol(buffer+2, NULL, 16));
                                   ListView_GetSubItemRect(listeWnd, i, 0, LVIR_BOUNDS, &rect);
                                   FillRect(NMCustomDraw->nmcd.hdc, &rect, brush);
                                   DeleteObject(brush);
                               }

                               return CDRF_NOTIFYSUBITEMDRAW;
                            }

                            return FALSE;
                       }
                }

                return FALSE;
           }
           case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                       case IDP_MODIFIER:
                       {
                            HWND listeWnd = GetDlgItem(hwnd, IDLV_LISTEFONCTIONS);
                            int numero = GetFirstSelectedItem(listeWnd);
                            if (numero < 0)
                            {
                               EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                               EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);
                               return FALSE;
                            }

                            Fonction fonction = RecupererFonction(listeWnd, numero),
                                     *ptFonction = NULL;

                            ptFonction = (Fonction*) DialogBoxParam(GetModuleHandle(NULL), "NouvelleFonction", hwnd, (DLGPROC)NouvelleFonction_proc, (LPARAM)&fonction);
                            if (!ptFonction)
                               return FALSE;

                            EntrerFonction(listeWnd, numero, *ptFonction);
                            ListView_DeleteItem(listeWnd, numero+1);

                            ListView_SetItemState(listeWnd, numero, LVIS_SELECTED, LVIS_SELECTED);
                            EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), TRUE);
                            EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), TRUE);

                            free(ptFonction);
                            return FALSE;
                       }
                       case IDP_AJOUTER:
                       {
                            HWND listeWnd = GetDlgItem(hwnd, IDLV_LISTEFONCTIONS);
                            if (ListView_GetItemCount(listeWnd) >= MAX_FONCTIONS)
                            {
                               MessageBox(hwnd, "Nombre maximal de fonctions atteint !", "Attention", MB_OK | MB_ICONWARNING);
                               return FALSE;
                            }

                            Fonction *fonction = (Fonction*) DialogBox(GetModuleHandle(NULL), "NouvelleFonction", hwnd, (DLGPROC)NouvelleFonction_proc);
                            if (!fonction)
                               return FALSE;

                            EntrerFonction(listeWnd, GetFirstSelectedItem(listeWnd), *fonction);

                            ListView_SetItemState(listeWnd, -1, 0, LVIS_SELECTED);
                            EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);

                            free(fonction);
                            return FALSE;
                       }
                       case IDP_SUPPRIMER:
                       {
                            HWND listeWnd = GetDlgItem(hwnd, IDLV_LISTEFONCTIONS);
                            int numero = GetFirstSelectedItem(listeWnd);
                            if (numero >= 0)
                               ListView_DeleteItem(listeWnd, numero);

                            if (numero >= ListView_GetItemCount(listeWnd) || numero < 0)
                            {
                                EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                                EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);
                            }
                            else ListView_SetItemState(listeWnd, numero, LVIS_SELECTED, LVIS_SELECTED);

                            return FALSE;
                       }
                       case IDP_RESET:
                            if (MessageBoxA(hwnd, "Voulez-vous vraiment supprimer toutes les fonctions définies ?", "Attention", MB_YESNO | MB_ICONWARNING) == IDYES)
                            {
                               ListView_DeleteAllItems(GetDlgItem(hwnd, IDLV_LISTEFONCTIONS));
                               EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                               EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);
                            }
                            return FALSE;
                }
    }

    return FALSE;
}

LRESULT CALLBACK OptionsVariables_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "X";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                RECT taille;
                GetClientRect(hwnd, &taille);
                SendMessage(GetDlgItem(hwnd, IDGB_VARIABLES), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);

                HWND listeWnd = CreateWindowEx(
                     WS_EX_CLIENTEDGE,
                     WC_LISTVIEW,
                     "",
                     WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHAREIMAGELISTS | LVS_NOSORTHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
                     15,
                     30,
                     taille.right-30,
                     taille.bottom-85,
                     hwnd,
                     (HMENU) IDLV_LISTEVARIABLES,
                     GetModuleHandle(NULL),
                     NULL
                );
                ListView_SetExtendedListViewStyle(listeWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

                LVCOLUMN lvcol;
                lvcol.mask = LVCF_WIDTH | LVCF_TEXT;
                lvcol.pszText = buffer;
                strcpy(buffer, "Nom");
                lvcol.cx = 70;
                ListView_InsertColumn(listeWnd, 0, &lvcol);

                lvcol.cx = taille.right-235;
                strcpy(buffer, "Expression littérale");
                ListView_InsertColumn(listeWnd, 1, &lvcol);

                lvcol.cx = 130;
                strcpy(buffer, "Valeur approchée");
                ListView_InsertColumn(listeWnd, 2, &lvcol);

                listeVariables_Wnd = listeWnd;

                EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);
                return FALSE;
           }
           case WM_NOTIFY:
           {
                NMHDR *nmhdr = (NMHDR*)lParam;
                switch (nmhdr->idFrom)
                {
                       case IDLV_LISTEVARIABLES:
                       {
                            if (nmhdr->code == NM_CLICK)
                            {
                               NMITEMACTIVATE *nmia = (NMITEMACTIVATE*)lParam;
                               int etat = 1;

                               if (nmia->iItem < 0)
                                  etat = 0;

                               EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), etat);
                               EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), etat);
                            }

                            if (nmhdr->code == NM_DBLCLK)
                            {
                               NMITEMACTIVATE *nmia = (NMITEMACTIVATE*)lParam;
                               if (nmia->iItem >= 0)
                                  SendMessage(hwnd, WM_COMMAND, IDP_MODIFIER, 0);
                            }

                            if (nmhdr->code == LVN_KEYDOWN)
                            {
                               NMLVKEYDOWN *nmlvkd = (NMLVKEYDOWN*)lParam;
                               if (nmlvkd->wVKey == VK_RETURN)
                                  SendMessage(hwnd, WM_COMMAND, IDP_MODIFIER, 0);
                               else if (nmlvkd->wVKey == VK_DELETE)
                                  SendMessage(hwnd, WM_COMMAND, IDP_SUPPRIMER, 0);
                            }

                            return FALSE;
                       }
                }

                return FALSE;
           }
           case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                       case IDP_MODIFIER:
                       {
                            HWND listeWnd = GetDlgItem(hwnd, IDLV_LISTEVARIABLES);
                            int numero = GetFirstSelectedItem(listeWnd);
                            if (numero < 0)
                            {
                               EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                               EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);
                               return FALSE;
                            }

                            Variable variable = RecupererVariable(listeWnd, numero),
                                     *ptVariable = NULL;

                            ptVariable = (Variable*) DialogBoxParam(GetModuleHandle(NULL), "NouvelleVariable", hwnd, (DLGPROC)NouvelleVariable_proc, (LPARAM)&variable);
                            if (!ptVariable)
                               return FALSE;

                            EntrerVariable(listeWnd, numero, *ptVariable);
                            ListView_DeleteItem(listeWnd, numero+1);

                            ListView_SetItemState(listeWnd, numero, LVIS_SELECTED, LVIS_SELECTED);
                            EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), TRUE);
                            EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), TRUE);

                            free(ptVariable);
                            return FALSE;
                       }
                       case IDP_AJOUTER:
                       {
                            HWND listeWnd = GetDlgItem(hwnd, IDLV_LISTEVARIABLES);
                            if (ListView_GetItemCount(listeWnd) >= MAX_VARIABLES)
                            {
                               MessageBox(hwnd, "Nombre maximal de variables atteint !", "Attention", MB_OK | MB_ICONWARNING);
                               return FALSE;
                            }

                            Variable *variable = (Variable*) DialogBox(GetModuleHandle(NULL), "Nouvellevariable", hwnd, (DLGPROC)NouvelleVariable_proc);
                            if (!variable)
                               return FALSE;

                            EntrerVariable(listeWnd, GetFirstSelectedItem(listeWnd), *variable);

                            ListView_SetItemState(listeWnd, -1, 0, LVIS_SELECTED);
                            EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                            EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);

                            free(variable);
                            return FALSE;
                       }
                       case IDP_SUPPRIMER:
                       {
                            HWND listeWnd = GetDlgItem(hwnd, IDLV_LISTEVARIABLES);
                            int numero = GetFirstSelectedItem(listeWnd);
                            if (numero >= 0)
                               ListView_DeleteItem(listeWnd, numero);

                            if (numero >= ListView_GetItemCount(listeWnd) || numero < 0)
                            {
                                EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                                EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);
                            }
                            else ListView_SetItemState(listeWnd, numero, LVIS_SELECTED, LVIS_SELECTED);

                            return FALSE;
                       }
                       case IDP_RESET:
                            if (MessageBoxA(hwnd, "Voulez-vous vraiment supprimer toutes les variables définies ?", "Attention", MB_YESNO | MB_ICONWARNING) == IDYES)
                            {
                               ListView_DeleteAllItems(GetDlgItem(hwnd, IDLV_LISTEVARIABLES));
                               EnableWindow(GetDlgItem(hwnd, IDP_SUPPRIMER), FALSE);
                               EnableWindow(GetDlgItem(hwnd, IDP_MODIFIER), FALSE);
                            }
                            return FALSE;
                }
    }

    return FALSE;
}



LRESULT CALLBACK OptionsAffichage_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
        return FALSE;
}


LRESULT CALLBACK Rappels_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                RECT taille;
                GetClientRect(hwnd, &taille);
                SendMessage(GetDlgItem(hwnd, IDGB_OBJETS), WM_SETFONT, (WPARAM)policeSpeciale, TRUE);

                listeRappelsFonctions_Wnd = CreateWindowEx(
                     WS_EX_CLIENTEDGE,
                     WC_LISTVIEW,
                     "",
                     WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL | LVS_SORTASCENDING,
                     15,
                     55,
                     taille.right-30,
                     taille.bottom-70,
                     hwnd,
                     (HMENU) IDLV_LISTEFONCTIONS,
                     GetModuleHandle(NULL),
                     NULL
                );
                ListView_SetExtendedListViewStyle(listeRappelsFonctions_Wnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

                LVCOLUMN lvcol;
                lvcol.mask = LVCF_WIDTH | LVCF_TEXT;
                lvcol.pszText = buffer;
                lvcol.cx = 90;
                strcpy(buffer, "Appel");
                ListView_InsertColumn(listeRappelsFonctions_Wnd, 0, &lvcol);

                lvcol.cx = taille.right-245;
                strcpy(buffer, "Description");
                ListView_InsertColumn(listeRappelsFonctions_Wnd, 1, &lvcol);

                lvcol.cx = 130;
                strcpy(buffer, "Domaine");
                ListView_InsertColumn(listeRappelsFonctions_Wnd, 2, &lvcol);

                int i = 1;
                LVITEM lvi;
                lvi.mask = LVIF_TEXT;
                lvi.pszText = buffer;

                for (i=0 ; tabFonctionsDefaut[i].description[0] ; i++)
                {
                       lvi.iItem = i;
                       lvi.iSubItem = 0;
                       strcpy(buffer, tabFonctionsDefaut[i].appel);
                       lvi.iItem = ListView_InsertItem(listeRappelsFonctions_Wnd, &lvi);

                       lvi.iSubItem++;
                       strcpy(buffer, tabFonctionsDefaut[i].description);
                       ListView_SetItem(listeRappelsFonctions_Wnd, &lvi);

                       lvi.iSubItem++;
                       strcpy(buffer, tabFonctionsDefaut[i].ensemble);
                       ListView_SetItem(listeRappelsFonctions_Wnd, &lvi);
                }


                listeRappelsVariables_Wnd = CreateWindowEx(
                     WS_EX_CLIENTEDGE,
                     WC_LISTVIEW,
                     "",
                     WS_CHILD | LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL | LVS_SORTASCENDING,
                     15,
                     55,
                     taille.right-30,
                     taille.bottom-70,
                     hwnd,
                     (HMENU) IDLV_LISTEVARIABLES,
                     GetModuleHandle(NULL),
                     NULL
                );
                ListView_SetExtendedListViewStyle(listeRappelsVariables_Wnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

                lvcol.cx = 70;
                strcpy(buffer, "Nom");
                ListView_InsertColumn(listeRappelsVariables_Wnd, 0, &lvcol);

                lvcol.cx = taille.right-235;
                strcpy(buffer, "Description");
                ListView_InsertColumn(listeRappelsVariables_Wnd, 1, &lvcol);

                lvcol.cx = 130;
                strcpy(buffer, "Valeur approchée");
                ListView_InsertColumn(listeRappelsVariables_Wnd, 2, &lvcol);

                for (i=0 ; tabVariablesDefaut[i].description[0] ; i++)
                {
                       lvi.iItem = i-1;
                       lvi.iSubItem = 0;
                       strncpy(buffer, tabVariablesDefaut[i].nom, MAX_CHAINE);
                       lvi.iItem = ListView_InsertItem(listeRappelsVariables_Wnd, &lvi);

                       lvi.iSubItem++;
                       strncpy(buffer, tabVariablesDefaut[i].description, MAX_CHAINE);
                       ListView_SetItem(listeRappelsVariables_Wnd, &lvi);

                       lvi.iSubItem++;
                       if (tabVariablesDefaut[i].expression[0])
                          tabVariablesDefaut[i].valeur = Calculer(tabVariablesDefaut[i].expression, '?', 0, &optionsAlgo);
                       sprintf(buffer, "%.4f", tabVariablesDefaut[i].valeur);
                       ListView_SetItem(listeRappelsVariables_Wnd, &lvi);
                }

                CheckDlgButton(hwnd, IDR_FONCTIONS, BST_CHECKED);
                return FALSE;
           }
           case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                       case IDR_VARIABLES:
                       case IDR_FONCTIONS:
                            ShowWindow(listeRappelsFonctions_Wnd, SW_HIDE);
                            ShowWindow(listeRappelsVariables_Wnd, SW_HIDE);

                            if (IsDlgButtonChecked(hwnd, IDR_VARIABLES))
                               ShowWindow(listeRappelsVariables_Wnd, SW_SHOW);
                            else ShowWindow(listeRappelsFonctions_Wnd, SW_SHOW);

                            return FALSE;
                }
                return FALSE;
    }
    return FALSE;
}



LRESULT CALLBACK Couleur_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paintst;
    HDC hdc;
    HBRUSH brush;
    RECT rect;

    switch (msg)
    {
           case WM_PAINT:
                hdc = BeginPaint(hwnd, &paintst);

                if (hwnd == carreCouleurTrace_Wnd)
                   brush = CreateSolidBrush(couleurTrace);
                else if (hwnd == carreCouleurRepere_Wnd)
                     brush = CreateSolidBrush(couleurRepere);
                else brush = CreateSolidBrush(couleurAnnexe);

                GetClientRect(hwnd, &rect);
                FillRect(hdc, &rect, brush);
                DeleteObject(brush);

                EndPaint(hwnd, &paintst);
                return FALSE;
    }

    return FALSE;
}

LRESULT CALLBACK NouvelleFonction_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                CentrerFenetre(hwnd, GetParent(hwnd));
                Fonction *fonction = (Fonction*)lParam;

                if (fonction)
                   SetWindowText(hwnd, "Modifier une fonction");

                HWND comboBox = GetDlgItem(hwnd, IDCB_TYPECOURBE);
                ComboBox_AddString(comboBox, "Cartésienne");
                ComboBox_AddString(comboBox, "Paramétrée");
                ComboBox_AddString(comboBox, "Polaire");
                ComboBox_AddString(comboBox, "Paramétrée polaire");
                if (fonction)
                   ComboBox_SetCurSel(comboBox, fonction->type);
                else ComboBox_SetCurSel(comboBox, 0);

                comboBox = GetDlgItem(hwnd, IDCB_VARIABLE);
                ComboBox_AddString(comboBox, "x");
                ComboBox_AddString(comboBox, "t");
                ComboBox_AddString(comboBox, "o");
                if (fonction)
                {
                   buffer[0] = fonction->variable;
                   ComboBox_SetText(comboBox, buffer);
                }
                else ComboBox_SetCurSel(comboBox, 0);
                SendMessage(comboBox, CB_LIMITTEXT, 1, 0);

                if (fonction)
                   couleurAnnexe = fonction->couleur;
                else couleurAnnexe = 0xFF;
                carreCouleur_Wnd = CreateDialog(GetModuleHandle(NULL), "Couleur", hwnd, (DLGPROC)Couleur_proc);
                SetWindowPos(carreCouleur_Wnd, NULL, 70, 228, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);

                sprintf(buffer, "0x%06X", (unsigned int)couleurAnnexe);
                SetDlgItemText(hwnd, IDT_COULEUR, buffer);

                if (fonction)
                   sprintf(buffer, "%.2f", fonction->borne1);
                else strcpy(buffer, "-10.00");
                SetDlgItemText(hwnd, IDE_BORNE1, buffer);
                if (fonction)
                   sprintf(buffer, "%.2f", fonction->borne2);
                else strcpy(buffer, "10.00");
                SetDlgItemText(hwnd, IDE_BORNE2, buffer);

                if (fonction)
                {
                   SetDlgItemText(hwnd, IDE_NOM, fonction->nom);
                   SetDlgItemText(hwnd, IDE_FONCTION, fonction->expression);
                }

                return FALSE;
           }
           case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                       case IDP_COULEUR:
                       {
                            CHOOSECOLOR cl;
                            COLORREF couleur;
                            memset(&cl, 0, sizeof(CHOOSECOLOR));

                            cl.lStructSize = sizeof(CHOOSECOLOR);
                            cl.hwndOwner = hwnd;
                            cl.lpCustColors = &couleur;
                            cl.Flags = CC_FULLOPEN;
                            if (ChooseColor(&cl))
                            {
                                sprintf(buffer, "0x%06X", (unsigned int)cl.rgbResult);
                                couleurAnnexe = cl.rgbResult;
                                SetDlgItemText(hwnd, IDT_COULEUR, buffer);
                                InvalidateRect(carreCouleur_Wnd, NULL, FALSE);
                                UpdateWindow(carreCouleur_Wnd);
                            }

                            return FALSE;
                       }
                       case IDP_OK:
                       {
                            Fonction *fonction = malloc(sizeof(Fonction));
                            Fonction *tab[MAX_FONCTIONS] = {NULL};
                            char expression[MAX_CHAINE];

                            if (!fonction)
                               return FALSE;
                            int i;

                            fonction->type = ComboBox_GetCurSel(GetDlgItem(hwnd, IDCB_TYPECOURBE));
                            GetDlgItemText(hwnd, IDE_NOM, fonction->nom, 25);

                            if (!fonction->nom[0])
                            {
                               MessageBox(hwnd, "Veuillez spécifier un nom de fonction.", "Attention", MB_OK | MB_ICONWARNING);
                               free(fonction);
                               return FALSE;
                            }

                            for (i = 0 ; fonction->nom[i] ; i++)
                            {
                                if (!TestLettre(fonction->nom[i]) && !TestNombre(fonction->nom[i]))
                                {
                                   MessageBox(hwnd, "Nom de fonction invalide !", "Attention", MB_OK | MB_ICONWARNING);
                                   free(fonction);
                                   return FALSE;
                                }
                            }

                            GetWindowText(hwnd, buffer, MAX_CHAINE);
                            if (strcmp("Modifier une fonction", buffer) && FonctionExiste(listeFonctions_Wnd, fonction->nom))
                            {
                               MessageBox(hwnd, "Fonction portant ce nom déjà existante !", "Attention", MB_OK | MB_ICONWARNING);
                               free(fonction);
                               return FALSE;
                            }

                            GetDlgItemText(hwnd, IDE_FONCTION, fonction->expression, MAX_CHAINE);
                            strncpy(expression, fonction->expression, MAX_CHAINE);

                            MettreAJourTabFonctions(listeFonctions_Wnd, tab, MAX_FONCTIONS);
                            i=0;
                            while (i < MAX_FONCTIONS && tab[i] && strcmp(tab[i]->nom, fonction->nom))
                                  i++;
                            if (i < MAX_FONCTIONS && tab[i])
                               strcpy(tab[i]->expression, expression);

                            GetDlgItemText(hwnd, IDCB_VARIABLE, buffer, 10);
                            if (!TestLettre(buffer[0]))
                            {
                               MessageBox(hwnd, "Variable invalide !", "Attention", MB_OK | MB_ICONWARNING);
                               free(fonction);
                               return FALSE;
                            }
                            fonction->variable = buffer[0];

                            if (ExpliciterExpression(expression, MAX_CHAINE, fonction->variable, tab, NULL, MAX_FONCTIONS, 0) >= CODE_ERREUR)
                            {
                               MessageBox(hwnd, "Trop de niveaux de récursion !\nVeuillez vérifier que votre fonction ne s'appelle pas elle-même.", "Attention", MB_OK | MB_ICONWARNING);
                               free(fonction);
                               return FALSE;
                            }

                            GetDlgItemText(hwnd, IDE_BORNE1, buffer, MAX_CHAINE);
                            fonction->borne1 = strtod(buffer, NULL);
                            GetDlgItemText(hwnd, IDE_BORNE2, buffer, MAX_CHAINE);
                            fonction->borne2 = strtod(buffer, NULL);

                            if (fonction->borne1 > fonction->borne2)
                            {
                               double temp = fonction->borne1;
                               fonction->borne1 = fonction->borne2;
                               fonction->borne2 = temp;
                            }

                            double test;
                            if (fonction->type == PARAMETREE || fonction->type == PARAMPOLAIRE)
                            {
                               char equation1[MAX_CHAINE] = "",
                                    equation2[MAX_CHAINE] = "";

                               if (!RecupererEquasParam(expression, equation1, equation2, fonction->variable, MAX_CHAINE))
                                  test = ERR_SYNTAXE;
                               else
                               {
                                   double test1 = Calculer(equation1, fonction->variable, (fonction->borne1+fonction->borne2)/2, &optionsAlgo),
                                          test2 = Calculer(equation2, fonction->variable, (fonction->borne1+fonction->borne2)/2, &optionsAlgo);
                                   if ((test1 >= CODE_ERREUR && test1 != ERR_MATH) || (test2 >= CODE_ERREUR && test2 != ERR_MATH))
                                      test = ERR_SYNTAXE;
                                   else test = 0;
                               }
                            }
                            else test = Calculer(expression, fonction->variable, (fonction->borne1+fonction->borne2)/2, &optionsAlgo);

                            if (test >= CODE_ERREUR && test != ERR_MATH)
                            {
                               MessageBox(hwnd, "Expression de fonction, ou variable de fonction, incorrecte !", "Attention", MB_OK | MB_ICONWARNING);
                               free(fonction);
                               return FALSE;
                            }

                            fonction->couleur = couleurAnnexe;
                            fonction->active = 1;

                            EndDialog(hwnd, (int)fonction);
                            return FALSE;
                       }
                       case IDCB_TYPECOURBE:
                       {
                            if (HIWORD(wParam) != CBN_SELCHANGE)
                               return FALSE;

                            int sel = ComboBox_GetCurSel(GetDlgItem(hwnd, IDCB_TYPECOURBE));
                            HWND comboBox = GetDlgItem(hwnd, IDCB_VARIABLE);
                            if (sel != 3)
                               ComboBox_SetCurSel(comboBox, sel);
                            else ComboBox_SetCurSel(comboBox, 1);

                            SetFocus(GetDlgItem(hwnd, IDP_OK));
                            return FALSE;
                       }
                       case IDP_ANNULER:
                            EndDialog(hwnd, 0);
                            return FALSE;
                }
                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}

LRESULT CALLBACK NouvelleVariable_proc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    char buffer[MAX_CHAINE] = "";

    switch (msg)
    {
           case WM_INITDIALOG:
           {
                CentrerFenetre(hwnd, GetParent(hwnd));
                Variable *variable = (Variable*)lParam;

                if (variable)
                {
                   SetWindowText(hwnd, "Modifier une variable");
                   SetDlgItemText(hwnd, IDE_NOM, variable->nom);
                   SetDlgItemText(hwnd, IDE_VARIABLE, variable->expression);
                }

                return FALSE;
           }
           case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                       case IDP_OK:
                       {
                            Variable *variable = malloc(sizeof(Variable));
                            Variable *tab[MAX_VARIABLES] = {NULL};
                            char expression[MAX_CHAINE];

                            if (!variable)
                               return FALSE;
                            int i;

                            GetDlgItemText(hwnd, IDE_NOM, variable->nom, 25);
                            if (!variable->nom[0])
                            {
                               MessageBox(hwnd, "Veuillez spécifier un nom de variable.", "Attention", MB_OK | MB_ICONWARNING);
                               free(variable);
                               return FALSE;
                            }

                            for (i = 0 ; variable->nom[i] ; i++)
                            {
                                if (!TestLettre(variable->nom[i]) && !TestNombre(variable->nom[i]))
                                {
                                   MessageBox(hwnd, "Nom de variable invalide !", "Attention", MB_OK | MB_ICONWARNING);
                                   free(variable);
                                   return FALSE;
                                }
                            }

                            GetWindowText(hwnd, buffer, MAX_CHAINE);
                            if (strcmp("Modifier une variable", buffer) && VariableExiste(listeVariables_Wnd, variable->nom))
                            {
                               MessageBox(hwnd, "Variable portant ce nom déjà existante !", "Attention", MB_OK | MB_ICONWARNING);
                               free(variable);
                               return FALSE;
                            }

                            GetDlgItemText(hwnd, IDE_VARIABLE, variable->expression, MAX_CHAINE);
                            strncpy(expression, variable->expression, MAX_CHAINE);

                            MettreAJourTabVariables(listeVariables_Wnd, tab, MAX_FONCTIONS);
                            i=0;
                            while (i < MAX_VARIABLES && tab[i] && strcmp(tab[i]->nom, variable->nom))
                                  i++;
                            if (i < MAX_VARIABLES && tab[i])
                               strcpy(tab[i]->expression, expression);

                            if (ExpliciterExpression(expression, MAX_CHAINE, '?', NULL, tab, 0, MAX_VARIABLES) >= CODE_ERREUR)
                            {
                               MessageBox(hwnd, "Trop de niveaux de récursion !\nVeuillez vérifier que votre variable est indépendante d'elle-même.", "Attention", MB_OK | MB_ICONWARNING);
                               free(variable);
                               return FALSE;
                            }

                            double test = Calculer(expression, '?', 0, &optionsAlgo);

                            if (test >= CODE_ERREUR && test != ERR_MATH)
                            {
                               MessageBox(hwnd, "Expression de variable incorrecte !", "Attention", MB_OK | MB_ICONWARNING);
                               free(variable);
                               return FALSE;
                            }

                            EndDialog(hwnd, (int)variable);
                            return FALSE;
                       }
                       case IDP_ANNULER:
                            EndDialog(hwnd, 0);
                            return FALSE;
                }
                return FALSE;
           case WM_CLOSE:
                EndDialog(hwnd, 0);
                return FALSE;
    }

    return FALSE;
}

DWORD WINAPI ThreadFenetreOptions(LPVOID lpParameter)
{
   return DialogBox(GetModuleHandle(NULL), "Options", NULL, (DLGPROC)Options_proc);
}

void CentrerFenetre(HWND hwnd1, HWND hwnd2)
{
     RECT screen;
     screen.right = GetSystemMetrics(SM_CXFULLSCREEN);
     screen.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
     screen.top = 0;
     screen.left = 0;

     if (hwnd2 != NULL)
     {
          RECT rect;
          GetWindowRect(hwnd2, &rect);
          GetClientRect(hwnd2, &screen);
          screen.top = rect.top;
          screen.left = rect.left;
     }

     RECT wndRect;
     GetWindowRect(hwnd1, &wndRect);

     SetWindowPos(hwnd1, NULL, (screen.right - (wndRect.right - wndRect.left)) / 2 + screen.left, (screen.bottom - (wndRect.bottom - wndRect.top)) / 2 + screen.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

     return;
}


void InitialiserVariables()
{
     int i;

     policeSpeciale = CreateFont(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "MS Sans Serif");

     optionsTrace.nombrePoints = 1000;
     optionsTrace.relierPoints = 1;
     optionsTrace.testDiscontinuite = 1,
     optionsTrace.couleurTrace = 0x000000FF;
     optionsTrace.couleurRepere = 0x00FF0000;
     optionsTrace.xmin = -6.3;
     optionsTrace.xmax = 6.3;
     optionsTrace.ymin = -3.1;
     optionsTrace.ymax = 3.1;
     optionsTrace.xech = 1;
     optionsTrace.yech = 1;
     optionsTrace.rotation = 0;
     optionsTrace.yorigine = 0;
     optionsTrace.xorigine = 0;
     optionsTrace.typeCourbe = CARTESIENNE;
     optionsTrace.variable = 'x';

     for (i = 0 ; i < 4 ; i++)
     {
         optionsParam[i].bornesAuto = 1;
         optionsParam[i].continuerTrace = 1;
         optionsParam[i].arreterTrace = 0;
         optionsParam[i].pointsConsecutifs = 0;
         optionsParam[i].nombrePointsSup = 500;
         optionsParam[i].nombrePointsDehors = 500;
         optionsParam[i].borne1 = -10;
         optionsParam[i].borne2 = 10;
         optionsParam[i].pasFixe = 0;
         optionsParam[i].pas = 0.01;
     }
     optionsParam[0].continuerTrace = 0;

     for (i = 0 ; i < 4 ; i++)
         optionsParamTemp[i] = optionsParam[i];

     optionsAlgo.nombrePointsP = 300;
     optionsAlgo.nombrePointsI = 300;
     optionsAlgo.calculIntg = RIEMANN;
     optionsAlgo.calculPrimitive = INTEGRATION;
     Calculer("1", '?', 0, &optionsAlgo); //envoi des paramètres d'algorithme à la section "équation"

     iconeVide = LoadIcon(GetModuleHandle(NULL), "icoVide");

     tabFonctionsDefaut = RecupererTabFnsDef();
     tabVariablesDefaut = RecupererTabVarDef();
     tabFonctions = RecupererTabFns();
     tabVariables = RecupererTabVar();

     SDL_Color violet = {255,0,255};
     SDL_Color rouge = {255,0,0};
     SDL_Color bleu = {0,0,255};

     boutonApparition.img = IMG_Load("fleche_gauche.png");
     boutonApparition.clStd = bleu;
     boutonApparition.clClic = violet;
     boutonApparition.dim.y = 3;
     boutonApparition.dim.w = boutonApparition.img->w;
     boutonApparition.dim.h = boutonApparition.img->h;
     boutonDisparition.img = IMG_Load("fleche_droite.png");
     boutonDisparition.clStd = bleu;
     boutonDisparition.clClic = violet;
     boutonDisparition.dim.y = 3;
     boutonDisparition.dim.w = boutonDisparition.img->w;
     boutonDisparition.dim.h = boutonDisparition.img->h;

     menu[0].img = IMG_Load("menu1.png");
     menu[0].nb = 6;
     menu[0].btn = malloc(sizeof(Bouton)*6);
     for (i=0 ; i<6 ; i++)
     {
         menu[0].btn[i].clStd = rouge;
         menu[0].btn[i].clClic = violet;
         menu[0].btn[i].dim.w = 50;
         menu[0].btn[i].dim.h = 50;
     }
     menu[0].btn[0].dim.x = 15;
     menu[0].btn[0].dim.y = 8;
     menu[0].btn[0].dim.h = 57;
     menu[0].btn[0].id = IDMENUZOOM;
     menu[0].btn[1].dim.x = 14;
     menu[0].btn[1].dim.y = 74;
     menu[0].btn[1].id = IDMENUAFF;
     menu[0].btn[2].dim.x = 13;
     menu[0].btn[2].dim.y = 138;
     menu[0].btn[2].id = IDREGLAGES;
     menu[0].btn[3].dim.x = 15;
     menu[0].btn[3].dim.y = 207;
     menu[0].btn[3].id = IDMENUGRAPH;
     menu[0].btn[4].dim.x = 14;
     menu[0].btn[4].dim.y = 274;
     menu[0].btn[4].dim.h = 52;
     menu[0].btn[4].id = IDMENUALG;
     menu[0].btn[5].dim.x = 17;
     menu[0].btn[5].dim.y = 343;
     menu[0].btn[5].id = IDMENUTAB;

     menu[1].img = IMG_Load("menuzoom.png");
     menu[1].nb = 7;
     menu[1].btn = malloc(sizeof(Bouton)*7);
     for (i=0 ; i<7 ; i++)
     {
         menu[1].btn[i].clStd = rouge;
         menu[1].btn[i].clClic = violet;
         menu[1].btn[i].dim.w = 50;
         menu[1].btn[i].dim.h = 50;
     }
     menu[1].btn[0].dim.x = 26;
     menu[1].btn[0].dim.y = 7;
     menu[1].btn[0].id = IDZOOMCARRE;
     menu[1].btn[1].dim.x = 23;
     menu[1].btn[1].dim.y = 70;
     menu[1].btn[1].dim.w = 24;
     menu[1].btn[1].dim.h = 24;
     menu[1].btn[1].id = IDZOOMPLUS;
     menu[1].btn[2].dim.x = 56;
     menu[1].btn[2].dim.y = 69;
     menu[1].btn[2].dim.w = 24;
     menu[1].btn[2].dim.h = 24;
     menu[1].btn[2].id = IDZOOMMOINS;
     menu[1].btn[3].dim.x = 22;
     menu[1].btn[3].dim.y = 130;
     menu[1].btn[3].dim.w = 60;
     menu[1].btn[3].dim.h = 23;
     menu[1].btn[3].id = IDZOOMAUTO;
     menu[1].btn[4].dim.x = 23;
     menu[1].btn[4].dim.y = 155;
     menu[1].btn[4].dim.w = 30;
     menu[1].btn[4].dim.h = 15;
     menu[1].btn[4].id = IDZOOMNORM;
     menu[1].btn[5].dim.x = 61;
     menu[1].btn[5].dim.y = 155;
     menu[1].btn[5].dim.w = 19;
     menu[1].btn[5].dim.h = 15;
     menu[1].btn[5].id = IDZOOMDEF;
     menu[1].btn[6].dim.x = 26;
     menu[1].btn[6].dim.y = 178;
     menu[1].btn[6].id = IDZOOMREGLAGES;

     menu[2].img = IMG_Load("menugraph.png");
     menu[2].nb = 6;
     menu[2].btn = malloc(sizeof(Bouton)*6);
     for (i=0 ; i<6 ; i++)
     {
         menu[2].btn[i].clStd = rouge;
         menu[2].btn[i].clClic = violet;
     }
     menu[2].btn[0].dim.x = 25;
     menu[2].btn[0].dim.y = 24;
     menu[2].btn[0].dim.w = 59;
     menu[2].btn[0].dim.h = 22;
     menu[2].btn[0].id = IDGRAPHISCT;
     menu[2].btn[1].dim.x = 23;
     menu[2].btn[1].dim.y = 80;
     menu[2].btn[1].dim.w = 60;
     menu[2].btn[1].dim.h = 22;
     menu[2].btn[1].id = IDGRAPHMAX;
     menu[2].btn[2].dim.x = 28;
     menu[2].btn[2].dim.y = 112;
     menu[2].btn[2].dim.w = 51;
     menu[2].btn[2].dim.h = 21;
     menu[2].btn[2].id = IDGRAPHMIN;
     menu[2].btn[3].dim.x = 19;
     menu[2].btn[3].dim.y = 162;
     menu[2].btn[3].dim.w = 71;
     menu[2].btn[3].dim.h = 22;
     menu[2].btn[3].id = IDGRAPHRESOLY;
     menu[2].btn[4].dim.x = 18;
     menu[2].btn[4].dim.y = 192;
     menu[2].btn[4].dim.w = 72;
     menu[2].btn[4].dim.h = 22;
     menu[2].btn[4].id = IDGRAPHRESOLX;
     menu[2].btn[5].dim.x = 28;
     menu[2].btn[5].dim.y = 248;
     menu[2].btn[5].dim.w = 50;
     menu[2].btn[5].dim.h = 31;
     menu[2].btn[5].id = IDGRAPHITEG;


     return;
}

int ReglerFenetreOptionsParam(OptionsParam options[])
{
     char buffer[MAX_CHAINE] = "";
     if (!optionsParam_Wnd)
        return 0;

     //remise à zéro de la fenêtre
     CheckDlgButton(optionsParam_Wnd, IDR_AUTOVARIATION, BST_UNCHECKED);
     CheckDlgButton(optionsParam_Wnd, IDR_MANUVARIATION, BST_UNCHECKED);
     CheckDlgButton(optionsParam_Wnd, IDC_CONTINUERTRACE, BST_UNCHECKED);
     CheckDlgButton(optionsParam_Wnd, IDC_ARRETERTRACE, BST_UNCHECKED);
     CheckDlgButton(optionsParam_Wnd, IDC_PASFIXE, BST_UNCHECKED);
     CheckDlgButton(optionsParam_Wnd, IDC_POINTSDEHORSCONSECUTIFS, BST_UNCHECKED);
     EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_BORNE1), TRUE);
     EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_BORNE2), TRUE);
     EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_POINTSSUP), TRUE);
     EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_POINTSDEHORS), TRUE);
     EnableWindow(GetDlgItem(optionsParam_Wnd, IDC_POINTSDEHORSCONSECUTIFS), TRUE);
     EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_PAS), TRUE);

     int i = ComboBox_GetCurSel(GetDlgItem(optionsParam_Wnd, IDCB_TYPECOURBE))+1;

     if (options[i].bornesAuto)
     {
        CheckDlgButton(optionsParam_Wnd, IDR_AUTOVARIATION, BST_CHECKED);
        EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_BORNE1), FALSE);
        EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_BORNE2), FALSE);
     }
     else CheckDlgButton(optionsParam_Wnd, IDR_MANUVARIATION, BST_CHECKED);

     if (options[i].continuerTrace)
        CheckDlgButton(optionsParam_Wnd, IDC_CONTINUERTRACE, BST_CHECKED);
     else EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_POINTSSUP), FALSE);

     if (options[i].arreterTrace)
        CheckDlgButton(optionsParam_Wnd, IDC_ARRETERTRACE, BST_CHECKED);
     else
     {
         EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_POINTSDEHORS), FALSE);
         EnableWindow(GetDlgItem(optionsParam_Wnd, IDC_POINTSDEHORSCONSECUTIFS), FALSE);
     }

     if (options[i].pasFixe)
        CheckDlgButton(optionsParam_Wnd, IDC_PASFIXE, BST_CHECKED);
     else EnableWindow(GetDlgItem(optionsParam_Wnd, IDE_PAS), FALSE);

     if (options[i].pointsConsecutifs)
        CheckDlgButton(optionsParam_Wnd, IDC_POINTSDEHORSCONSECUTIFS, BST_CHECKED);

     sprintf(buffer, "%d", options[i].nombrePointsDehors);
     SetDlgItemText(optionsParam_Wnd, IDE_POINTSDEHORS, buffer);
     sprintf(buffer, "%d", options[i].nombrePointsSup);
     SetDlgItemText(optionsParam_Wnd, IDE_POINTSSUP, buffer);
     sprintf(buffer, "%.2f", options[i].borne1);
     SetDlgItemText(optionsParam_Wnd, IDE_BORNE1, buffer);
     sprintf(buffer, "%.2f", options[i].borne2);
     SetDlgItemText(optionsParam_Wnd, IDE_BORNE2, buffer);
     sprintf(buffer, "%.2f", options[i].pas);
     SetDlgItemText(optionsParam_Wnd, IDE_PAS, buffer);

     return 1;
}

int SauverOptionsParam(OptionsParam *options)
{
    char buffer[MAX_CHAINE] = "";
    if (!optionsParam_Wnd)
       return 0;

    options->bornesAuto = IsDlgButtonChecked(optionsParam_Wnd, IDR_AUTOVARIATION);
    options->continuerTrace = IsDlgButtonChecked(optionsParam_Wnd, IDC_CONTINUERTRACE);
    options->arreterTrace = IsDlgButtonChecked(optionsParam_Wnd, IDC_ARRETERTRACE);
    options->pasFixe = IsDlgButtonChecked(optionsParam_Wnd, IDC_PASFIXE);
    options->pointsConsecutifs = IsDlgButtonChecked(optionsParam_Wnd, IDC_POINTSDEHORSCONSECUTIFS);

    GetDlgItemText(optionsParam_Wnd, IDE_BORNE1, buffer, MAX_CHAINE);
    options->borne1 = strtod(buffer, NULL);
    GetDlgItemText(optionsParam_Wnd, IDE_BORNE2, buffer, MAX_CHAINE);
    options->borne2 = strtod(buffer, NULL);
    GetDlgItemText(optionsParam_Wnd, IDE_PAS, buffer, MAX_CHAINE);
    options->pas = strtod(buffer, NULL);
    GetDlgItemText(optionsParam_Wnd, IDE_POINTSSUP, buffer, MAX_CHAINE);
    options->nombrePointsSup = strtol(buffer, NULL, 10);
    GetDlgItemText(optionsParam_Wnd, IDE_POINTSDEHORS, buffer, MAX_CHAINE);
    options->nombrePointsDehors = strtol(buffer, NULL, 10);

    if (!options->bornesAuto && options->borne1 >= options->borne2)
    {
       MessageBox(optionsParam_Wnd, "Bornes de paramétrage invalides !", "Attention", MB_OK | MB_ICONWARNING);
       return 0;
    }

    if (options->pasFixe && options->pas <= 0)
    {
       MessageBox(optionsParam_Wnd, "Pas de paramétrage invalide !", "Attention", MB_OK | MB_ICONWARNING);
       return 0;
    }

    if (options->continuerTrace && options->nombrePointsSup >= 10000 && MessageBox(optionsParam_Wnd, "Le nombre de points supplémentaires est très grand. Ceci peut ralentir les calculs.\nContinuer ?", "Attention", MB_YESNO | MB_ICONWARNING) == IDNO)
       return 0;

    return 1;
}



int IsItemSelected(HWND listWnd, int item)
{
    if (!listWnd || item < 0)
       return -1;

    int state = LOBYTE(ListView_GetItemState(listWnd, item, LVIS_SELECTED));
    if ((state & LVIS_SELECTED) == LVIS_SELECTED)
       return 1;
    else return 0;
}


int GetFirstSelectedItem(HWND listWnd)
{
    if (!listWnd)
       return -1;

    int count = ListView_GetItemCount(listWnd),
        i;

    for (i = 0 ; i < count ; i++)
    {
        if (IsItemSelected(listWnd, i))
           return i;
    }

    return count;
}


Fonction RecupererFonction(HWND listeWnd, int item)
{
         Fonction fonction;
         memset(&fonction, 0, sizeof(Fonction));
         char *position = NULL,
              buffer[MAX_CHAINE] = "";

         if (!listeWnd || item < 0)
            return fonction;

         ListView_GetItemText(listeWnd, item, 1, buffer, MAX_CHAINE);
         if ( (position = strchr(buffer, '(')) )
            fonction.variable = *(position+1);
         if ( (position = strchr(buffer, '=')) )
         {
            strncpy(fonction.nom, buffer, 20);
            fonction.nom[position-buffer-1] = '\0';
            strncpy(fonction.expression, position+2, MAX_CHAINE);
            if ( (position = strchr(fonction.nom, '(')) )
               *position = '\0';
         }

         ListView_GetItemText(listeWnd, item, 2, buffer, MAX_CHAINE);
         fonction.borne1 = strtod(buffer+1, &position);
         if (position)
            fonction.borne2 = strtod(position+3, NULL);

         ListView_GetItemText(listeWnd, item, 3, buffer, MAX_CHAINE);
         if (!strcmp(buffer, "Paramétrée polaire"))
            fonction.type = PARAMPOLAIRE;
         else if (!strcmp(buffer, "Paramétrée"))
              fonction.type = PARAMETREE;
         else if (!strcmp(buffer, "Polaire"))
              fonction.type = POLAIRE;
         else fonction.type = CARTESIENNE;

         ListView_GetItemText(listeWnd, item, 4, buffer, MAX_CHAINE);
         fonction.couleur = strtol(buffer+2, NULL, 16);
         fonction.active = ListView_GetCheckState(listeWnd, item);

         return fonction;
}

int EntrerFonction(HWND listeWnd, int item, Fonction fonction)
{
    if (!listeWnd || item < 0)
       return 0;

    LVITEM lvi;
    char buffer[MAX_CHAINE] = "";

    lvi.mask = LVIF_TEXT;
    lvi.iItem = item;
    lvi.iSubItem = 0;
    lvi.pszText = "";
    ListView_InsertItem(listeWnd, &lvi);

    lvi.iSubItem++;
    lvi.pszText = buffer;
    sprintf(buffer, "%s(%c) = %s", fonction.nom, fonction.variable, fonction.expression);
    ListView_SetItem(listeWnd, &lvi);

    lvi.iSubItem++;
    if (fonction.borne1 == fonction.borne2)
       strcpy(buffer, "R");
    else sprintf(buffer, "[%.2f ; %.2f]", fonction.borne1, fonction.borne2);
         ListView_SetItem(listeWnd, &lvi);

    lvi.iSubItem++;
    strcpy(buffer, "Inconnue !");
    if (fonction.type == CARTESIENNE)
       strcpy(buffer, "Cartésienne");
    else if (fonction.type == PARAMETREE)
         strcpy(buffer, "Paramétrée");
    else if (fonction.type == POLAIRE)
         strcpy(buffer, "Polaire");
    else if (fonction.type == PARAMPOLAIRE)
         strcpy(buffer, "Paramétrée polaire");

    ListView_SetItem(listeWnd, &lvi);

    lvi.iSubItem++;
    sprintf(buffer, "0x%06X", (unsigned int)fonction.couleur);
    ListView_SetItem(listeWnd, &lvi);

    ListView_SetCheckState(listeWnd, item, fonction.active);

    return 1;
}

int MettreAJourTabFonctions(HWND listeWnd, Fonction *tab[], int taille)
{
    if (!tab)
    {
       tab = tabFonctions;
       taille = MAX_FONCTIONS;
    }
    if (!listeWnd || taille <= 0)
       return 0;

    int i, j = 0,
        nombreItems = ListView_GetItemCount(listeWnd);

    for (i = 0 ; i < taille ; i++)   //remise à zéro du tableau
    {
        if (tab[i])
           free(tab[i]);
        tab[i] = NULL;
    }

    for (i = 0 ; i < nombreItems ; i++)  //enregistrement du nouveau tableau
    {
        tab[j] = malloc(sizeof(Fonction));
        if (tab[j])
        {
           *(tab[j]) = RecupererFonction(listeWnd, i);
           j++;
        }
    }

    return 1;
}

int MettreAJourTabVariables(HWND listeWnd, Variable *tab[], int taille)
{
    if (!tab)
    {
       tab = tabVariables;
       taille = MAX_VARIABLES;
    }
    if (!listeWnd || taille <= 0)
       return 0;

    int i, j = 0,
        nombreItems = ListView_GetItemCount(listeWnd);

    for (i = 0 ; i < taille ; i++)   //remise à zéro du tableau
    {
        if (tab[i])
           free(tab[i]);
        tab[i] = NULL;
    }

    for (i = 0 ; i < nombreItems ; i++)  //enregistrement du nouveau tableau
    {
        tab[j] = malloc(sizeof(Variable));
        if (tab[j])
        {
           *(tab[j]) = RecupererVariable(listeWnd, i);
           j++;
        }
    }

    return 1;
}


int FonctionExiste(HWND listeWnd, char nom[])
{
    int i,
        nombreItems = MAX_FONCTIONS;
    Fonction fonction;

    if (listeWnd)
       nombreItems = ListView_GetItemCount(listeWnd);

    for (i = 0 ; i < nombreItems ; i++)  //enregistrement du nouveau tableau
    {
        if (listeWnd)
           fonction = RecupererFonction(listeWnd, i);
        else if (tabFonctions[i])
             fonction = *(tabFonctions[i]);
        else fonction.nom[0] = '\0';

        if (!strcmp(fonction.nom, nom))
           return 1;
    }

    return 0;
}

int VariableExiste(HWND listeWnd, char nom[])
{
    int i,
        nombreItems = MAX_FONCTIONS;
    Variable variable;

    if (listeWnd)
       nombreItems = ListView_GetItemCount(listeWnd);

    for (i = 0 ; i < nombreItems ; i++)  //enregistrement du nouveau tableau
    {
        if (listeWnd)
           variable = RecupererVariable(listeWnd, i);
        else if (tabVariables[i])
             variable = *(tabVariables[i]);
        else variable.nom[0] = '\0';

        if (!strcmp(variable.nom, nom))
           return 1;
    }

    return 0;
}


Variable RecupererVariable(HWND listeWnd, int item)
{
         Variable variable;
         memset(&variable, 0, sizeof(Variable));
         char buffer[MAX_CHAINE] = "";

         if (!listeWnd || item < 0)
            return variable;

         ListView_GetItemText(listeWnd, item, 0, buffer, MAX_CHAINE);
         strncpy(variable.nom, buffer, 25);

         ListView_GetItemText(listeWnd, item, 1, buffer, MAX_CHAINE);
         strcpy(variable.expression, buffer);

         return variable;
}

int EntrerVariable(HWND listeWnd, int item, Variable variable)
{
    if (!listeWnd || item < 0)
       return 0;

    LVITEM lvi;
    char buffer[MAX_CHAINE] = "";

    lvi.mask = LVIF_TEXT;
    lvi.iItem = item;
    lvi.iSubItem = 0;
    lvi.pszText = buffer;
    strcpy(buffer, variable.nom);
    ListView_InsertItem(listeWnd, &lvi);

    lvi.iSubItem++;
    strcpy(buffer, variable.expression);
    ListView_SetItem(listeWnd, &lvi);


    Variable *tab[MAX_VARIABLES] = {NULL};
    MettreAJourTabVariables(listeVariables_Wnd, tab, MAX_FONCTIONS);
    ExpliciterExpression(buffer, MAX_CHAINE, '?', NULL, tab, 0, MAX_VARIABLES);

    lvi.iSubItem++;
    double resultat = Calculer(buffer, '?', 0, &optionsAlgo);
    if (resultat < CODE_ERREUR)
       sprintf(buffer, "%.4f", resultat);
    else strcpy(buffer, "###");
    ListView_SetItem(listeWnd, &lvi);

    return 1;
}


int ChangerMenu(Menu *m)
{
    if (timerActif)
        SDL_RemoveTimer(timerId);
    else timerActif = 1;

    if (!m)
    {
        if (!menuActuel)
            return 0;
        menuActuel->pos.x = LARGEUR-menuActuel->img->w;
        menuActuel->pos.y = 0;
        timerId = SDL_AddTimer(30, TransitionDroite, menuActuel);
    }
    else
    {
        if (!menuActuel)
        {
            ancienMenu = NULL;
            m->pos.x = LARGEUR;
            m->pos.y = 0;
            menuActuel = m;
            timerId = SDL_AddTimer(30, TransitionGauche, m);
        }
        else
        {
            Menu **tab = malloc(2*sizeof(Menu*));
            tab[0]=menuActuel;
            tab[1]=m;
            m->pos.x = LARGEUR-1;
            m->pos.y = 0;
            menuActuel->pos.x = LARGEUR-menuActuel->img->w;
            menuActuel->pos.y = 0;

            ancienMenu = menuActuel;
            menuActuel = m;
            timerId = SDL_AddTimer(30, Fondu, tab);
        }
    }
    return 1;
}


Uint32 TransitionDroite(Uint32 interval, void* param)
{
    Menu *m = (Menu*)param;
    m->pos.x += interval*m->img->w/500.0;

    if (m->pos.x >= LARGEUR)
    {
        m->pos.x = LARGEUR;
        SDL_RemoveTimer(timerId);
        timerActif = 0;
    }

    DeclencherEvent();
    return 1;
}

Uint32 TransitionGauche(Uint32 interval, void* param)
{
    Menu *m = (Menu*)param;
    m->pos.x -= interval*m->img->w/500.0;

    if (m->pos.x <= LARGEUR-m->img->w)
    {
        m->pos.x = LARGEUR-m->img->w;
        SDL_RemoveTimer(timerId);
        timerActif = 0;
    }

    DeclencherEvent();
    return 1;
}

Uint32 Fondu(Uint32 interval, void* param)
{
    Menu **tab = (Menu**)param;
    tab[0]->pos.x += interval*tab[0]->img->w/500.0;
    tab[1]->pos.x -= interval*tab[1]->img->w/500.0;

    if (tab[1]->pos.x <= LARGEUR-tab[1]->img->w)
       tab[1]->pos.x = LARGEUR-tab[1]->img->w;
    if (tab[0]->pos.x >= LARGEUR)
       tab[0]->pos.x = LARGEUR;

    if (tab[0]->pos.x == LARGEUR && tab[1]->pos.x == LARGEUR-tab[1]->img->w)
    {
        SDL_RemoveTimer(timerId);
        timerActif = 0;
        free(tab);
    }

    DeclencherEvent();
    return 1;
}


void DeclencherEvent()
{
    SDL_Event event;
    memset(&event, 0, sizeof(SDL_Event));
    SDL_PushEvent(&event);
    return;
}


int DessinerTexte(SDL_Surface *s, SDL_Rect pos, char texte[], TTF_Font *police, SDL_Color clPP, SDL_Color clAP, int type)
{
    if (!s || !texte || !police)
        return 0;

    SDL_Surface *surfaceTexte = NULL;

    switch (type)
    {
        case SOLID:
             surfaceTexte = TTF_RenderText_Solid(police, texte, clPP);
             break;
        case BLENDED:
             surfaceTexte = TTF_RenderText_Blended(police, texte, clPP);
             break;
        case SHADED:
             surfaceTexte = TTF_RenderText_Shaded(police, texte, clPP, clAP);
             break;
    }

    if (!surfaceTexte)
        return 0;

    SDL_BlitSurface(surfaceTexte, NULL, s, &pos);
    return 1;
}

void AjouterCoordonnees(SDL_Surface *s, TTF_Font *police)
{
    SDL_Rect pos = {5,5};
    char tampon[MAX_CHAINE];
    double X, Y;
    POINT souris;
    SDL_Color rouge = {255,0,0};
    Uint8 *etatClavier = SDL_GetKeyState(NULL);
    SDL_GetMouseState((int*)&(souris.x), (int*)&(souris.y));
    PixelVersCoord(souris, optionsTrace, surfaceCourbe->w, surfaceCourbe->h, &X, &Y);

    if (etatClavier[SDLK_LSHIFT])
       sprintf(tampon, "(%.3f ; %.3f)", round(X), round(Y));
    else sprintf(tampon, "(%.3f ; %.3f)", X, Y);

    DessinerTexte(s, pos, tampon, police, rouge, rouge, SOLID);

    return;
}

SDL_Rect PositionnerSecondCurseur(SDL_Surface *s)
{
    Uint8 *etatClavier = SDL_GetKeyState(NULL);
    POINT souris;
    SDL_GetMouseState((int*)&(souris.x), (int*)&(souris.y));
    SDL_Rect pos = {souris.x, souris.y};
    if (!etatClavier[SDLK_LSHIFT])
        return pos;

    double X, Y;
    PixelVersCoord(souris, optionsTrace, surfaceCourbe->w, surfaceCourbe->h, &X, &Y);
    POINT arrondi = CoordVersPixel(round(X), round(Y), optionsTrace, surfaceCourbe->w, surfaceCourbe->h);
    pos.x = arrondi.x;
    pos.y = arrondi.y;

    return pos;
}

//int SelectionnerCourbe(SDL_Event event
