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

#include "affichage_texte.h"


int TEXTE_Afficher (SDL_Surface **surfaceBlit,
                    SDL_Rect *positionBlit,
                    char texte[],
                    int typeBlit,
                    SDL_Color couleurPP,
                    SDL_Color couleurAP,
                    int espaceH,
                    int espaceV,
                    int flags,
                    TTF_Font *tableauPolices[],
                    int nombrePolices)

{
    /*
    Affiche un texte complexe avec passages à la ligne et balises de type BBCode dans une surface donnée.
    Le mode d'emploi est disponible dans un fichier texte en annexe.
    */


    //verification des paramètres passés
    if (!surfaceBlit || (!*surfaceBlit && (!positionBlit || positionBlit->w <= 0  || positionBlit->h <= 0)) || !texte || !tableauPolices || !tableauPolices[0])
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    //ajustement des paramètres
    SDL_Rect positionBlit2 = {0};
    if (!positionBlit)
       positionBlit = &positionBlit2;  //pour éviter un malloc
    SDL_Color couleurAlpha = couleurAP;  //génération de la couleur qui servira de transparence : couleurAP par défaut, mais ne doit pas être égale à la couleurPP
    if (typeBlit != TEXTE_BLIT_SHADED && couleurAP.r == couleurPP.r && couleurAP.g == couleurPP.g && couleurAP.b == couleurPP.b)
       couleurAlpha.r += 10;
    if (!*surfaceBlit)
    {
       *surfaceBlit = SDL_CreateRGBSurface(SDL_HWSURFACE, positionBlit->w + positionBlit->x, positionBlit->h + positionBlit->y, 32, 0, 0, 0, 0);
       if (!*surfaceBlit)
          return TEXTE_ERREUR_SDL;
       SDL_FillRect(*surfaceBlit, 0, SDL_MapRGB((*surfaceBlit)->format, couleurAlpha.r, couleurAlpha.g, couleurAlpha.b));
       /*if (typeBlit != TEXTE_BLIT_SHADED)
          SDL_SetColorKey(*surfaceBlit, SDL_SRCCOLORKEY, SDL_MapRGB((*surfaceBlit)->format, couleurAlpha.r, couleurAlpha.g, couleurAlpha.b));*/  //surface transparent si non shaded
    }
    else if (positionBlit->w <= 0)
       positionBlit->w = (*surfaceBlit)->w - positionBlit->x;
    if (nombrePolices <= 0)
       nombrePolices = 1;

    //création des différentes surfaces
    int hauteurMaxi = TEXTE_HauteurMaxi(tableauPolices[0], texte);
    SDL_Surface *surfaceMot = SDL_CreateRGBSurface(SDL_HWSURFACE, positionBlit->w, hauteurMaxi, 32, 0, 0, 0, 0);
    if (!surfaceMot)
       return TEXTE_ERREUR_SDL;
    SDL_FillRect(surfaceMot, 0, SDL_MapRGB(surfaceMot->format, couleurAlpha.r, couleurAlpha.g, couleurAlpha.b));
    /*if (typeBlit != TEXTE_BLIT_SHADED)
       SDL_SetColorKey(surfaceMot, SDL_SRCCOLORKEY, SDL_MapRGB(surfaceMot->format, couleurAlpha.r, couleurAlpha.g, couleurAlpha.b));*/

    SDL_Surface *surfaceLigne = SDL_CreateRGBSurface(SDL_HWSURFACE, positionBlit->w, hauteurMaxi, 32, 0, 0, 0, 0);
    if (!surfaceLigne)
       return TEXTE_ERREUR_SDL;
    SDL_FillRect(surfaceLigne, 0, SDL_MapRGB(surfaceLigne->format, couleurAlpha.r, couleurAlpha.g, couleurAlpha.b));
    /*if (typeBlit != TEXTE_BLIT_SHADED)
       SDL_SetColorKey(surfaceLigne, SDL_SRCCOLORKEY, SDL_MapRGB(surfaceLigne->format, couleurAlpha.r, couleurAlpha.g, couleurAlpha.b));*/

    //mesure de la longueur des espaces
    int i, *longueurEspace = malloc(nombrePolices * sizeof(int));
    if (!longueurEspace)
       return TEXTE_ERREUR_ALLOC;
    SDL_Surface *surfaceEspace = NULL;
    for (i = 0 ; i < nombrePolices ; i++)
    {
        surfaceEspace = TTF_RenderText_Solid(tableauPolices[i], " ", couleurPP);
        if (surfaceEspace)
        {
           longueurEspace[i] = surfaceEspace->w;
           SDL_FreeSurface(surfaceEspace);
           surfaceEspace = NULL;
        }
    }

    //initialisation des variables
    SDL_Rect positionBlitLigne = *positionBlit,
             positionBlitMot = {0},
             positionBlitLettre = {0};
    SDL_Surface *surfaceLettre = NULL;
    SDL_Color couleurPPActuelle = couleurPP,
              couleurAPActuelle = couleurAP;
    int continuer = 1,
        baliseTrouvee,
        espaceHActuel = espaceH,
        styleActuel = TTF_STYLE_NORMAL,
        numeroPolice = 0,
        longueurMot = 0,
        longueurLigne = 0,
        motTermine = 0,
        alignementDroit = 0,
        centre = 0;
    hauteurMaxi = 0;
    i = 0;
    char tampon[2] = {0};


    //boucle d'écriture
    while (continuer)
    {
          //contrôle du BBCode
          baliseTrouvee = 0;
          if (texte[i] == '[' && flags & TEXTE_FLAGS_BALISES)
          {
             char *offset = NULL;
             int numero = 0;

             switch (texte[i+1])
             {
                    case 'a':   //changement de couleur AP
                    case 'c':   //changement de couleur PP
                         if (texte[i+2] != '=' || texte[i+6] != ']')
                            break;

                         if (texte[i+1] == 'c')
                            TEXTE_ConvertirCouleur(texte+i+3, &couleurPPActuelle);
                         else TEXTE_ConvertirCouleur(texte+i+3, &couleurAPActuelle);

                         i += 6;
                         baliseTrouvee = 1;
                         break;
                    case 'e':   //espacement des caracteres
                         if (texte[i+2] != '=')
                            break;

                         numero = strtol(texte+i+3, &offset, 10);
                         if (*offset != ']')
                            break;
                         else espaceHActuel = numero;

                         i = (int)offset - (int)texte;
                         baliseTrouvee = 1;
                         break;
                    case 'g':   //gras
                         if (texte[i+2] != ']')
                            break;

                         styleActuel |= TTF_STYLE_BOLD;
                         i += 2;
                         baliseTrouvee = 1;
                         break;
                    case 'i':   //italique
                         if (texte[i+2] != ']')
                            break;

                         styleActuel |= TTF_STYLE_ITALIC;
                         i += 2;
                         baliseTrouvee = 1;
                         break;
                    case 'm':   //changement de mise en page
                         if (texte[i+2] != '=' || texte[i+4] != ']')
                            break;

                         if (texte[i+3] == 'c')
                         {
                            alignementDroit = 0;
                            centre = 1;
                         }
                         else if (texte[i+3] == 'd')
                         {
                              alignementDroit = 1;
                              centre = 0;
                         }
                         else break;
                         i += 4;
                         baliseTrouvee = 1;
                         break;
                    case 'p':   //changement de police
                         if (texte[i+2] != '=')
                            break;

                         numero = strtol(texte+i+3, &offset, 10);
                         if (*offset != ']')
                            break;
                         else numeroPolice = numero;

                         if (numeroPolice < 0 || numeroPolice >= nombrePolices || !tableauPolices[numeroPolice])
                            numeroPolice = 0;
                         i = (int)offset - (int)texte;
                         baliseTrouvee = 1;
                         break;
                    case 's':   //souligné
                         if (texte[i+2] != ']')
                            break;

                         styleActuel |= TTF_STYLE_UNDERLINE;
                         i += 2;
                         baliseTrouvee = 1;
                         break;
                    case '/':   //fermeture d'un code
                         if (texte[i+3] != ']')
                            break;

                         if (texte[i+2] == 'a')
                            couleurAPActuelle = couleurAP;
                         else if (texte[i+2] == 'c')
                            couleurPPActuelle = couleurPP;
                         else if (texte[i+2] == 'e')
                              espaceHActuel = espaceH;
                         else if (texte[i+2] == 'g' && styleActuel & TTF_STYLE_BOLD)
                              styleActuel -= TTF_STYLE_BOLD;
                         else if (texte[i+2] == 'i' && styleActuel & TTF_STYLE_ITALIC)
                              styleActuel -= TTF_STYLE_ITALIC;
                         else if (texte[i+2] == 'p')
                              numeroPolice = 0;
                         else if (texte[i+2] == 's' && styleActuel & TTF_STYLE_UNDERLINE)
                              styleActuel -= TTF_STYLE_UNDERLINE;

                         i += 3;
                         baliseTrouvee = 1;
                         break;
             }
          }

          if (baliseTrouvee)
             TTF_SetFontStyle(tableauPolices[numeroPolice], styleActuel);    //mise en place du style
          else if ((texte[i] == ' ' && !(TEXTE_FLAGS_TRONQUER & flags)) || texte[i] == '\0' || texte[i] == '\n' || motTermine)   //si le mot est terminé, ajout sur la ligne
          {
               motTermine = 0;
               int motBlitte = 0;

               if ((texte[i] == '\n' || texte[i] == '\0') && surfaceLigne->w >= longueurMot + positionBlitMot.x)   //avant un passage à la ligne, si le dernier mot rentre, l'ajouter
               {
                  motBlitte = 1;
                  SDL_BlitSurface(surfaceMot, NULL, surfaceLigne, &positionBlitMot);    //ajout du mot sur la ligne
                  //TEXTE_ShadedVersBlended(surfaceMot, NULL, surfaceLigne, &positionBlitMot);
                  positionBlitMot.x += longueurMot + 2*espaceHActuel + longueurEspace[numeroPolice];
                  longueurLigne = positionBlitMot.x - 2*espaceHActuel - longueurEspace[numeroPolice];
               }

               if (surfaceLigne->w < longueurMot + positionBlitMot.x || texte[i] == '\n' || texte[i] == '\0')   //si le mot ne rentre pas sur la ligne ou si retour à la ligne ou si fin du texte, passage à la suivante
               {
                  if (centre)   //ajustement de la position en fonction de l'alignement actuel du texte
                     positionBlitLigne.x += (positionBlit->w - longueurLigne) / 2;
                  else if (alignementDroit)
                       positionBlitLigne.x = positionBlit->w + positionBlit->x - longueurLigne;
                  //SDL_BlitSurface(surfaceLigne, NULL, *surfaceBlit, &positionBlitLigne);  //ajout de la ligne sur la surface
                  TEXTE_ShadedVersBlended(surfaceLigne, NULL, *surfaceBlit, &positionBlitLigne);

                  //remise à zéro des variables : passage à la ligne
                  positionBlitLigne.x = positionBlit->x;
                  positionBlitLigne.y += hauteurMaxi + espaceV;

                  if (!motBlitte && (texte[i] == '\n' || texte[i] == '\0') && surfaceLigne->w < longueurMot + positionBlitMot.x)
                  {
                     SDL_BlitSurface(surfaceMot, NULL, *surfaceBlit, &positionBlitLigne);
                     //TEXTE_ShadedVersBlended(surfaceMot, NULL, *surfaceBlit, &positionBlitLigne);
                     /*exceptionnellement, lorsqu'un passage à la ligne est demandé ET que le dernier mot ne rentre pas sur la ligne,
                     on autorise ce code pour faire deux passages à la lignes consécutifs*/
                     positionBlitLigne.y += hauteurMaxi + espaceV;
                     motBlitte = 1;
                  }
                  hauteurMaxi = 0;

                  //réinitialisation de la surface Ligne
                  SDL_FillRect(surfaceLigne, NULL, SDL_MapRGB(surfaceLigne->format, couleurAlpha.r, couleurAlpha.g, couleurAlpha.b));
                  positionBlitMot.x = 0;
               }

               if (!motBlitte)
               {
                  SDL_BlitSurface(surfaceMot, NULL, surfaceLigne, &positionBlitMot);    //ajout du mot sur la ligne
                  //TEXTE_ShadedVersBlended(surfaceMot, NULL, surfaceLigne, &positionBlitMot);
                  positionBlitMot.x += longueurMot + 2*espaceHActuel + longueurEspace[numeroPolice];
                  longueurLigne = positionBlitMot.x - 2*espaceHActuel - longueurEspace[numeroPolice];
               }

               //réinitialisation de la surface Mot
               SDL_FillRect(surfaceMot, NULL, SDL_MapRGB(surfaceMot->format, couleurAlpha.r, couleurAlpha.g, couleurAlpha.b));
               positionBlitLettre.x = 0;

               if (texte[i] == '\0')
                  continuer = 0;
          }
          else if (texte[i] != '\t' && texte[i] != '\r')   //ne pas prendre en compte les tabulations et autres caractères particuliers
          {
              tampon[0] = texte[i];

              if (surfaceLettre)
              {
                 SDL_FreeSurface(surfaceLettre);
                 surfaceLettre = NULL;
              }
              switch (typeBlit)   //création d'une lettre
              {
                     case TEXTE_BLIT_SHADED:
                          surfaceLettre = TTF_RenderText_Shaded(tableauPolices[numeroPolice], tampon, couleurPPActuelle, couleurAPActuelle);
                          break;
                     case TEXTE_BLIT_BLENDED:
                          surfaceLettre = TTF_RenderText_Blended(tableauPolices[numeroPolice], tampon, couleurPPActuelle);
                          break;
                     default:
                             surfaceLettre = TTF_RenderText_Solid(tableauPolices[numeroPolice], tampon, couleurPPActuelle);
                             break;
              }

              if (!(TEXTE_FLAGS_TRONQUER & flags) && surfaceLettre && surfaceLettre->w + positionBlitLettre.x > surfaceMot->w)  //si la lettre ne rentre pas dans le mot, on le coupe
              {
                 motTermine = 1;
                 i -= 2;
              }
              else if (surfaceLettre && surfaceLettre->w + positionBlitLettre.x <= surfaceMot->w)
              {
                   SDL_BlitSurface(surfaceLettre, NULL, surfaceMot, &positionBlitLettre);   //ajout de la lettre au mot
                   //TEXTE_ShadedVersBlended(surfaceLettre, NULL, surfaceMot, &positionBlitLettre);
                   positionBlitLettre.x += surfaceLettre->w + espaceHActuel;
                   longueurMot = positionBlitLettre.x - espaceHActuel;
                   if (surfaceLettre->h > hauteurMaxi)
                      hauteurMaxi = surfaceLettre->h;
              }
          }

          if (positionBlitLigne.y + hauteurMaxi > (*surfaceBlit)->h)   //si on dépasse la hauteur de la zone de blit, arrêter la boucle
             continuer = 0;
          i++;
    }

    free(longueurEspace);
    return 1;
}

int TEXTE_AfficherEx (AFFICHERTEXTE *at)
{
    /*
    fonction identique à TEXTE_Afficher mais destinée à faciliter l'entrée des paramètres
    */

    if (!at)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;
    else return TEXTE_Afficher(at->surfaceBlit,
                               at->positionBlit,
                               at->texte,
                               at->typeBlit,
                               at->couleurPP,
                               at->couleurAP,
                               at->espaceH,
                               at->espaceV,
                               at->flags,
                               at->tableauPolices,
                               at->nombrePolices);
}


int TEXTE_ConvertirCouleur(char texte[], SDL_Color *couleur)
{
          /*
          convertit une chaine représentant un nombre hexa au format 0x--- en couleur au format SDL_Color
          exemple : passer "f0f" retournera du violet {255,0,255}
          */

          //verification des paramètres passés
          if (!texte || !couleur)
             return TEXTE_ERREUR_PARAMETRE_INVALIDE;

          char couleurHexa[2] = {0};

          //conversion des nombres hexa en décimaux de 0 à 255
          couleurHexa[0] = texte[0];
          couleur->r = strtol(couleurHexa, NULL, 16) * 255 / 15;
          couleurHexa[0] = texte[1];
          couleur->g = strtol(couleurHexa, NULL, 16) * 255 / 15;
          couleurHexa[0] = texte[2];
          couleur->b = strtol(couleurHexa, NULL, 16) * 255 / 15;

          return 1;
}


int TEXTE_Tronquer(char texte[], int largeurMax, TTF_Font *police)
{
     if (!texte || largeurMax <= 0 || !police)
        return TEXTE_ERREUR_PARAMETRE_INVALIDE;

     int i = 0;
     SDL_Color couleurNoire = {0, 0, 0};

     SDL_Surface *surfaceTexte = TTF_RenderText_Blended(police, texte, couleurNoire);
     if (!surfaceTexte)
        return TEXTE_ERREUR_SDL;

     for (i = strlen(texte) ; surfaceTexte->w > largeurMax && i >= 3 ; i--)
     {
         if (i < 3)
            strcpy(texte, " ");
         else if (strlen(texte) > i)
              strcpy(texte+i-3, "...");

         SDL_FreeSurface(surfaceTexte);
         surfaceTexte = TTF_RenderText_Solid(police, texte, couleurNoire);
         if (!surfaceTexte)
            return TEXTE_ERREUR_SDL;
     }

     SDL_FreeSurface(surfaceTexte);
     return 1;
}


int TEXTE_ShadedVersBlended(SDL_Surface *surface1, SDL_Rect *positionSurface1, SDL_Surface *surface2, SDL_Rect *positionSurface2)
{
    //permet de blitter une surface shaded (sur fond noir) sur un fond autre en respectant la semi-transparence appliquée au noir

    if (!surface1 || !surface2)
       return TEXTE_ERREUR_PARAMETRE_INVALIDE;

    Uint32 pixel, pixel2;
    Uint8 r1,r2,g1,g2,b1,b2;
    int i,j;
    double k;
    SDL_Rect position1 = {0},
             position2 = {0};


    if (!positionSurface1)
    {
       position1.w = surface1->w;
       position1.h = surface1->h;
    }
    else position1 = *positionSurface1;

    if (!positionSurface1)
    {
       position2.w = surface2->w;
       position2.h = surface2->h;
    }
    else position2 = *positionSurface2;



    if (position1.w>surface1->w)
       position1.w = surface1->w;

    if (position2.w>surface2->w)
       position2.w = surface2->w;

    if (position1.h>surface1->h)
       position1.h = surface1->h;

    if (position2.h>surface2->h)
       position2.h = surface2->h;



    SDL_LockSurface(surface1);
    SDL_LockSurface(surface2);

    for (i = position1.x ; i < position1.w ; i++)
    {
        for (j = position1.y ; j < position1.h ; j++)
        {
            pixel = TEXTE_GetPixel(surface1, i, j);
            pixel2 = TEXTE_GetPixel(surface2, i-position1.x+position2.x, j-position1.y+position2.y);

            SDL_GetRGB(pixel, surface1->format, &r1, &g1, &b1);
            SDL_GetRGB(pixel2, surface2->format, &r2, &g2, &b2);

            k = r1/255.0;
            if (g1/255.0>k)
               k = g1/255.0;
            if (b1/255.0>k)
               k = b1/255.0;

            r1 = (k*r1+(1-k)*r2);
            g1 = (k*g1+(1-k)*g2);
            b1 = (k*b1+(1-k)*b2);

            pixel = SDL_MapRGB(surface2->format, r1, g1, b1);
            TEXTE_PutPixel(surface2, i-position1.x+position2.x, j-position1.y+position2.y, pixel);
        }
    }

    SDL_UnlockSurface(surface1);
    SDL_UnlockSurface(surface2);

    return 1;
}




Uint32 TEXTE_GetPixel(SDL_Surface *surface, int x, int y)  //de la doc de la SDL
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

void TEXTE_PutPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)  //de la doc de la SDL
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}
