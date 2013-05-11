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

#include "equation.h"


OptionsAlgo oa;

double Calculer (char *chaineDeCalcul, char variable, double valeur, OptionsAlgo *options)
{
    FonctionDefaut *tabFonctionsDefaut = RecupererTabFnsDef();

    if (options)
        oa = *options;

    if (!chaineDeCalcul)
       return ERR_PARAM;
    else if (!chaineDeCalcul[0])
         return ERR_SYNTAXE;


    int i,
        longueur = strlen(chaineDeCalcul),
        operateur = 0,
        sortie = 0,
        separation = 0;
    char *position = NULL,
         chaine[MAX_CHAINE] = "\0",
         tampon[MAX_CHAINE] = "\0";


    //copie de la chaine de cacul
    if (longueur >= MAX_CHAINE-100)
       return ERR_TROPLONG;
    else strcpy(chaine, chaineDeCalcul);

    //élimination des espaces
    for (i = 0 ; chaine[i] ; i++)
    {
        if (chaine[i] == ' ')
           strcpy(chaine+i, chaine+i+1);
    }
    longueur = strlen(chaine);

    //vérification syntaxique
    if (CompterCaracteres(chaine, '(') != CompterCaracteres(chaine, ')'))
       return ERR_SYNTAXE;

    //élimination des parenthèses parasites
    if (chaine[0] == '(' && chaine[longueur-1] == ')')
    {
       if (RechercheParenthese(chaine+1) == chaine+longueur-1)
       {
          chaine[longueur-1] = '\0';
          strcpy(chaine, chaine+1);
          return Calculer(chaine, variable, valeur, &oa);
       }
    }

    //recherche des opérateurs
    for (i = 0 ; chaine[i] && !sortie ; i++)
    {
        switch (chaine[i])
        {
               case '+':
                    if (i>0 && !TestOperateur(chaine[i-1]))
                    //différenciation du + d'opération de celui du signe
                    {
                       operateur = PLUS;
                       separation = i;
                       sortie = 1;
                    }
                    break;
               case '-':
                    if (i>0 && !TestOperateur(chaine[i-1]))
                    //différenciation du - d'opération de celui du signe
                    {
                       operateur = MOINS;
                       separation = i;
                    }
                    else if (!operateur || operateur == FACT || operateur == SGNM)
                    {
                         operateur = SGNM;
                         separation = 0;
                    }
                    break;
               case '*':
                    if (operateur != MOINS)
                    {
                       separation = i;
                       operateur = MULT;
                    }
                    break;
               case '/':
                    if (operateur != MOINS)
                    {
                       operateur = DIV;
                       separation = i;
                    }
                    break;
               case '^':
                    if (!operateur || operateur == FACT || operateur == PUISS)
                    {
                       operateur = PUISS;
                       separation = i;
                    }
                    break;
               case '!':
                    if (!operateur || operateur == FACT)
                    {
                       operateur = FACT;
                       separation = longueur-1;
                    }
                    break;
               case '(':
                    //on saute tout le contenu de la parenthèse
                    position = RechercheParenthese(chaine+i+1);
                    if (position)
                       i = position-chaine;
                    else return ERR_SYNTAXE;
                    break;
               case '#':
                    //multiplication implicite
                    if (operateur == PUISS || operateur == FACT || !operateur)
                    {
                       separation = i;
                       operateur = MULT;
                    }
                    break;
        }
    }


    //opération
    if (operateur)
    {
        chaine[separation] = '\0';
        separation++;

        double a = Calculer(chaine, variable, valeur, &oa),
               b = Calculer(chaine+separation, variable, valeur, &oa);
        if (a >= CODE_ERREUR && operateur != SGNM)  //en cas de code d'erreur
           return a;
        else if (b >= CODE_ERREUR && operateur != FACT)
        {
             return b;
        }

        switch (operateur)
        {
               case PLUS:
                    return a + b;
               case MOINS:
                    return a - b;
               case SGNM:
                    return -b;
               case MULT:
                    return a * b;
               case DIV:
                    if (!b)
                       return ERR_MATH;
                    else return a / b;
               case PUISS:
                    if (a<0 && floor(b) != b)
                       return ERR_MATH;
                    else return pow(a, b);
               case FACT:
                    if (floor(a) != a || a<0)
                       return ERR_MATH;
                    else return Factorielle(a);
        }
    }

    operateur = 0;
    separation = 0;


    int longueur2;
    double a;
    char buffer[MAX_CHAINE+1];

    //analyse des fonctions
    for (i = 0 ; tabFonctionsDefaut[i].description[0] ; i++)
    {
        longueur2 = strlen(tabFonctionsDefaut[i].nom);
        strncpy(tampon, chaine, longueur2+1);
        tampon[longueur2+1] = '\0';
        sprintf(buffer, "%s(", tabFonctionsDefaut[i].nom);

        if (!strcmp(tampon, buffer))
        {
           if (chaine[longueur-1] != ')')
              return ERR_SYNTAXE;

           if (tabFonctionsDefaut[i].fonction1)
           {
              a = Calculer(chaine+longueur2, variable, valeur, &oa);
              if (a >= CODE_ERREUR)
                 return a;
              else return tabFonctionsDefaut[i].fonction1(a);
           }
           else if (tabFonctionsDefaut[i].fonction2)
                return tabFonctionsDefaut[i].fonction2(chaine, variable, valeur, &oa);
        }
    }

    char *fin = chaine+1;
    double resultat = 0;

    //renvoi du résultat si expression simple
    if (chaine[0] == variable)
       resultat = valeur;
    else resultat = strtod(chaine, &fin);

    if (fin && *fin)
       return ERR_VAR;  //en cas de variable inconnue dans l'expression
    else return resultat;
}


int CompterCaracteres(char *chaine, char caractere)
{
    int i, compte = 0;
    for (i = 0 ; chaine[i] ; i++)
    {
        if (chaine[i] == caractere)
           compte++;
    }

    return compte;
}


int TestNombre(char caractere)
{
    if (caractere >= '0' && caractere <= '9')
       return 1;
    else return 0;
}

int TestLettre(char caractere)
{
    if ((caractere >= 'a' && caractere <= 'z') || (caractere >= 'A' && caractere <= 'Z'))
       return 1;
    else return 0;
}

int TestSymbole(char caractere)
{
    return !TestLettre(caractere) && !TestNombre(caractere);
}

int TestOperateur(char caractere)
{
    return caractere=='+' || caractere=='-' || caractere=='*' || caractere=='/' || caractere=='^' || caractere=='#';
}

int TestOperateurEx(char caractere)
{
    return TestOperateur(caractere) || caractere=='(' || caractere==')' || caractere==' ';
}


int Factorielle( int nombre)
{
    if (nombre<0)
       return ERR_MATH;
    if (!nombre)
       return 1;

    int i, resultat = nombre;
    for (i = 1 ; i < nombre ; i++)
        resultat *= nombre-i;

    return resultat;
}


int RecupererParametres(char chaine[], double param[], int max, char variable, double valeur, char *tab[], int tailleMax)
{
    //permet de récupérer les paramètres passés à une fonction
    //la chaine envoyée doit contenir les parenthèses d'appel

    if (!chaine || (!param && !tab))
       return 0;

    char *tampon = malloc(strlen(chaine)+1),
         *position = NULL;
    if (!tampon)
       return 0;
    else strcpy(tampon, chaine);

    if ( (position = strchr(tampon, '(')) )
       strcpy(tampon, position+1);
    if ( (position = strrchr(tampon, ')')) )
       *position = '\0';

    int i, sortie = 0;
    for (i = 0 ; i < max && !sortie ; i++)
    {
        position = RechercheSeparateur(tampon);
        if (!position)
        {
           if (tab && tab[i] && strlen(tampon) < tailleMax)
              strcpy(tab[i], tampon);
           if (param)
              param[i] = Calculer(tampon, variable, valeur, NULL);
           sortie = 1;
        }
        else
        {
            *position = '\0';
            if (tab && tab[i] && strlen(tampon) < tailleMax)
               strcpy(tab[i], tampon);
            if (param)
               param[i] = Calculer(tampon, variable, valeur, NULL);
            tampon = position+1;
        }
    }

    if (i+1<max && sortie && param)
       param[i+1] = ERR_VAR;

    free(tampon);
    return i;
}

double CalculSerie(double param[], int taille, int fonction)
{
      if (!param || taille <= 0)
         return ERR_PARAM;

      int i;
      double max = param[0]-1,
            min = param[0]+1,
            moy = 0,
            somme = 0,
            prod = 1;

      for (i = 0 ; i<taille ; i++)
      {
          if (param[i]>max)
             max = param[i];
          if (param[i]<min)
             min = param[i];
          somme += param[i];
          prod *= param[i];
      }
      moy = somme / taille;

      switch (fonction)
      {
             case MAX:
                  return max;
             case MIN:
                  return min;
             case MOY:
                  return moy;
             case SOMME:
                  return somme;
             case PROD:
                  return prod;
      }

      return ERR_PARAM;
}


int TestMultImp(char chaine[], int position, char variable)
{
    //fonction capable d'affirmer si oui ou non il y a multiplication implicite à un endroit donné de la chaine de calcul

    if (position<=0 || !chaine)
       return 0;

    if (chaine[position] == '(')
    {
       if (chaine[position-1] == ')' || chaine[position-1] == '!')
          return 1;
       if ((TestVariable(chaine, position-1) || TestNombre(chaine[position-1]) || chaine[position-1] == variable) && !TestFonction(chaine, position-1))
          return 1;
       return 0;
    }

    if ((chaine[position-1] == ')' || chaine[position-1] == '!') && (chaine[position] == '(' || !TestSymbole(chaine[position])))
       return 1;
    if (TestNombre(chaine[position]) && TestNombre(chaine[position-1]))
        return 0;

    if (!TestSymbole(chaine[position]) && !TestSymbole(chaine[position-1]))
    {
       int t1 = TestVariable(chaine, position),
           t2 = TestFonction(chaine, position);
       if (t2 == 1 || t1 == 1)
          return 0;
       else if (t1 || t2)
            return 1;

       if ((TestNombre(chaine[position]) || chaine[position] == variable) && (chaine[position-1] == variable || TestNombre(chaine[position-1]) || TestVariable(chaine, position-1)))
          return 1;
       return 0;
    }

    return 0;
}

int TestVariable(char chaine[], int position)
{
    if (!chaine || position < 0)
       return 0;

    int i,
        debut = position,
        fin,
        sortie = 0;

    if (position >= strlen(chaine) || (!TestLettre(chaine[position]) && !TestNombre(chaine[position])))
       return 0;

    for (i=position-1 ; i >= 0 && (TestLettre(chaine[i]) || TestNombre(chaine[i])) ; i--);
    debut = i+1;

    for (i=position+1 ; TestLettre(chaine[i]) || TestNombre(chaine[i]) ; i++);
    fin = i;

    char *cpChaine = malloc(fin-debut+1),
         *p = NULL;
    memset(cpChaine, 0, fin-debut+1);
    strncpy(cpChaine, chaine+debut, fin-debut);

    VariableDefaut *tab1 = RecupererTabVarDef();
    for (i=0 ; tab1[i].description[0] && sortie != 1 ; i++)
    {
        p = strstr(cpChaine, tab1[i].nom);
        if (p)
        {
           p += (char*)debut - cpChaine;
           if ((int)p < position && (int)(p+strlen(tab1[i].nom)) > position)
              sortie = 1;
           else if ((int)p == position)
                sortie = 2;
        }
    }

    Variable** tab2 = RecupererTabVar();
    for (i=0 ; i < MAX_VARIABLES && tab2[i] && sortie != 1 ; i++)
    {
        p = strstr(cpChaine, tab2[i]->nom);
        if (p)
        {
           p += (char*)debut - cpChaine;
           if ((int)p < position && (int)(p+strlen(tab2[i]->nom)) > position)
              sortie = 1;
           else if ((int)p == position)
                sortie = 2;
        }
    }

    free(cpChaine);
    return sortie;
}

int TestFonction(char chaine[], int position)
{
    if (!chaine || position < 0)
       return 0;

    int i,
        debut = position,
        fin,
        sortie = 0;

    if (position >= strlen(chaine) || (!TestLettre(chaine[position]) && !TestNombre(chaine[position])))
       return 0;

    for (i=position-1 ; i >= 0 && (TestLettre(chaine[i]) || TestNombre(chaine[i])); i--);
    debut = i+1;

    for (i=position+1 ; TestLettre(chaine[i]) || TestNombre(chaine[i]) ; i++);
    if (chaine[i] != '(')
       return 0;
    fin = i+1;

    char *cpChaine = malloc(fin-debut+1),
         cpFonction[20] = "",
         *p = NULL;
    memset(cpChaine, 0, fin-debut+1);
    strncpy(cpChaine, chaine+debut, fin-debut);

    FonctionDefaut* tab1 = RecupererTabFnsDef();
    for (i=0 ; tab1[i].description[0] && sortie != 1 ; i++)
    {
        sprintf(cpFonction, "%s(", tab1[i].nom);

        p = strstr(cpChaine, cpFonction);
        if (p)
        {
           p += (char*)debut - cpChaine;
           if ((int)p < position)
              sortie = 1;
           else if ((int)p == position)
                sortie = 2;
        }
    }

    Fonction** tab2 = RecupererTabFns();
    for (i=0 ; i < MAX_FONCTIONS && tab2[i] && sortie != 1 ; i++)
    {
        sprintf(cpFonction, "%s(", tab2[i]->nom);

        p = strstr(cpChaine, cpFonction);
        if (p)
        {
           p += (char*)debut - cpChaine;
           if ((int)p < position)
              sortie = 1;
           else if ((int)p == position)
                sortie = 2;
        }
    }

    free(cpChaine);
    return sortie;
}


char *RechercheParenthese(char *position)
{
     if (!position)
        return NULL;

     int i, nbP = 0;
     for (i = 0 ; *(position+i) && nbP >= 0 ; i++)
     {
         if (*(position+i) == '(')
            nbP++;
         else if (*(position+i) == ')')
            nbP--;
     }

     if (nbP >= 0)
        return NULL;
     else return position+i-1;
}

char *RechercheSeparateur(char *position)
{
     if (!position)
        return NULL;

     int i;
     for (i = 0 ; *(position+i) ; i++)
     {
         if (*(position+i) == ',')
            return position+i;
         else if (*(position+i) == '(')
         {
              char *pos = RechercheParenthese(position+i+1);
              if (!pos)
                 return NULL;
              else i = pos-position;
         }
     }

     return NULL;
}

double Derivee(char equation[], char variable, double valeur)
{
      double test = Calculer(equation, variable, valeur, NULL);
      if (test >= CODE_ERREUR)
         return test;

      double pas = 0.0001,
            a = Calculer(equation, variable, valeur+pas, NULL),
            b = Calculer(equation, variable, valeur-pas, NULL);

      if (a >= CODE_ERREUR)
         return a;
      if (b >= CODE_ERREUR)
         return b;
      return (a-b)/(2*pas);
}

double IntegrationAires(char equation[], char variable, double valeurDebut, double valeurFin, int nombrePoints)
{
      //calcul approximatif d'intégrale via les aires

      if (!equation)
         return ERR_PARAM;
      if (valeurDebut == valeurFin)
         return 0;

      double a = valeurDebut, b = valeurFin;
      int inverse = 0;
      if (b<a)
      {
         inverse = 1;
         b = valeurDebut;
         a = valeurFin;
      }

      double pas = (b-a)/nombrePoints,
             intg = 0,
             resultat1, resultat2, i;

      int discontinue = 0;
      for (i = a ; i <= b-pas && !discontinue ; i += pas)
      {
          resultat1 = Calculer(equation, variable, i, NULL);
          resultat2 = Calculer(equation, variable, i+pas, NULL);

          if (resultat1 >= CODE_ERREUR)
             discontinue = resultat1;
          else if (resultat2 >= CODE_ERREUR)
             discontinue = resultat2;
          else intg += (resultat1 + resultat2)/2.0 * pas;
      }

      if (discontinue)
         return discontinue;
      else if (inverse)
           return -intg;
      else return intg;
}

double Primitive(char equation[], char variable, double valeur, double a, double b, OptionsAlgo options)
{
       //calcul de primitive en laissant le choix de l'algorithme à utiliser
       if (options.calculPrimitive == EULER)
          return PrimitiveEuler(equation, variable, valeur, a, b, options.nombrePointsP);
       else return PrimitiveParIntegration(equation, variable, valeur, a, b, options);
}

double Integration(char equation[], char variable, double valeurDebut, double valeurFin, OptionsAlgo options)
{
       //calcul intégral en laissant le choix de l'algorithme à utiliser
       switch (options.calculIntg)
       {
              case AIRES:
                   return IntegrationAires(equation, variable, valeurDebut, valeurFin, options.nombrePointsI);
              case GAUSS:
                   return IntegrationGauss(equation, variable, valeurDebut, valeurFin);
              case PROBA:
                   return IntegrationProba(equation, variable, valeurDebut, valeurFin, options.nombrePointsI);
              case PRIMITIVE:
                   return IntegrationParPrimitive(equation, variable, valeurDebut, valeurFin, options.nombrePointsI);
              default:
                      return IntegrationRiemann(equation, variable, valeurDebut, valeurFin, options.nombrePointsI);
       }
}


double PrimitiveParIntegration(char equation[], char variable, double valeur, double a, double b, OptionsAlgo options)
{
      // on cherche la valeur pour x = valeur de la primtive de "équation" qui prend la valeur a en b
      // Rappel : F(x) - F(b) = Intg(f,b,x)

      double intg = Integration(equation, variable, b, valeur, options);
      if (intg >= CODE_ERREUR)
         return intg;
      else return intg + a;
}

double IntegrationParPrimitive(char equation[], char variable, double valeurDebut, double valeurFin, int nombrePoints)
{
       //integration par un simple calcul de primitive
       return PrimitiveEuler(equation, variable, valeurFin, 0, valeurDebut, nombrePoints);
}

double PrimitiveEuler(char equation[], char variable, double valeur, double a, double b, int nombrePoints)
{
      //idem que PrimitiveParIntegration() mais en utilisant la méthode d'Euler

      if (!equation)
         return ERR_PARAM;
      if (valeur == b)
         return a;

      double i,
             pas = (valeur-b)/nombrePoints,
             resultat = a,
             temp = 0;

      for (i=b ; (i<valeur && pas>0) || (i>valeur && pas<0) ; i+=pas)
      {
          temp = Calculer(equation, variable, i, NULL);
          if (temp >= CODE_ERREUR)
             return temp;
          resultat = pas*temp+resultat;
      }

      return resultat;
}

double IntegrationProba(char equation[], char variable, double valeurDebut, double valeurFin, int nombrePoints)
{
      //intégration par algorithme de probabilité

      if (!equation)
         return ERR_PARAM;
      if (valeurDebut == valeurFin)
         return 0;

      double hauteur,
             largeur = VAbsolue(valeurFin-valeurDebut),
             hauteur1 = VAbsolue(Calculer(equation, variable, valeurDebut, NULL)),
             hauteur2 = VAbsolue(Calculer(equation, variable, valeurFin, NULL));

      if (hauteur1 > hauteur2)
         hauteur = hauteur1;
      else hauteur = hauteur2;

      double x, y, temp;
      int i, somme = 0;
      for (i = 0 ; i < nombrePoints ; i++)
      {
          x = hauteur*(rand()%(nombrePoints+1))/nombrePoints;
          y = largeur*(rand()%(nombrePoints+1))/nombrePoints;

          temp = Calculer(equation, variable, x, NULL);
          if (temp >= CODE_ERREUR)
             return temp;

          if (y <= VAbsolue(temp))
             somme++;
      }

      return (somme*1.0/nombrePoints)*(largeur*hauteur);
}


double IntegrationGauss(char equation[], char variable, double valeurDebut, double valeurFin)
{
      //intégration par méthode de Gauss (polynômes de Legendre)

      double x[5] = {0};
      x[1] = (1.0/21)*(sqrt(245-14*sqrt(70)));
      x[2] = -x[1];
      x[3] = (1.0/21)*(sqrt(245-14*sqrt(70)));
      x[4] = -x[3];

      double w[5] = {0};
      w[0] = 128.0/255;
      w[1] = (1.0/900)*(322+13*sqrt(70));
      w[2] = w[1];
      w[3] = (1.0/900)*(322-13*sqrt(70));
      w[4] = w[3];

      int i;
      double intg = 0, temp;
      for (i = 0 ; i < 5 ; i++)
      {
          temp = Calculer(equation, variable, x[i]*(valeurFin-valeurDebut)/2.0+(valeurFin+valeurDebut)/2.0, NULL);
          if (temp >= CODE_ERREUR)
             return temp;

          intg += w[i]*temp;
      }

      return intg*(valeurFin-valeurDebut)/2.0;
}

double IntegrationRiemann(char equation[], char variable, double valeurDebut, double valeurFin, int nombrePoints)
{
      //intégration par somme de Riemman

      if (!equation)
         return ERR_PARAM;

      double test = TestContinuite(equation, variable, valeurDebut, valeurFin);
      if (test >= CODE_ERREUR)
         return CODE_ERREUR;
      else if (!test)
           return ERR_MATH;

      double a = valeurDebut, b = valeurFin;
      if (a==b)
         return 0;
      else if (b<a)
      {
           b = valeurDebut;
           a = valeurFin;
      }

      int i, n = nombrePoints;
      double intg = 0, temp;
      for (i = 0 ; i < n ; i++)
      {
          temp = Calculer(equation, variable, a+i*(b-a)/n, NULL);
          if (temp >= CODE_ERREUR)
             return temp;

          intg += temp;
      }

      if (valeurDebut>valeurFin)
         return -intg*(b-a)/n;
      else return intg*(b-a)/n;
}


double VAbsolue(double x)
{
      if (x < 0)
         return -x;
      else return x;
}

double TestContinuite(char equation[], char variable, double a, double b)
{
    //vérifie la continuité d'une fonction sur un intervalle donné [a;b]

    if (!equation)
       return ERR_PARAM;
    if (a==b)
       return ERR_MATH;

    double borne1 = a,
           borne2 = b;
    if (borne1>borne2)
    {
       borne1 = b;
       borne2 = a;
    }

    double pas = (borne2-borne1)/300.0,
           i = borne1,
           y2 = Calculer(equation, variable, borne1, NULL),
           y1;

    if (y2 >= CODE_ERREUR)
        return y2;

    for (; i <= borne2-pas ; i += pas)
    {
        y1 = y2;
        y2 = Calculer(equation, variable, i+pas, NULL);

        if (!TestContinuitePoint(equation, variable, i, i+pas, &y1, &y2))
           return 0;
    }

    return 1;
}


/*double TestContinuitePoint(char equation[], char variable, double a, double b)
{
       //teste la continuité rapidement entre deux points d'abscisse a et b

       if (!equation)
          return ERR_PARAM;
       if (a==b)
          return ERR_MATH;

       double c = Calculer(equation, variable, a, NULL),
              d = Calculer(equation, variable, b, NULL);

       if (c >= CODE_ERREUR)
            return c;
       else if (d >= CODE_ERREUR)
            return d;

       if ((c-d)/(a-b) > TAUX_VARIATION_MAX)
           return 0;
       else return 1;
}*/

double TestContinuitePoint(char equation[], char variable, double a, double b, double *y1, double *y2)
{
       //teste la continuité rapidement entre deux points d'abscisse a et b

       if (!equation)
          return ERR_PARAM;
       if (a==b)
          return ERR_MATH;

       double c = (a+b)/2.0, d, e,
              f = Calculer(equation, variable, c, NULL),
              tv1, tv2;

       if (y1)
          d = *y1;
       else d = Calculer(equation, variable, a, NULL);

       if (y2)
          e = *y2;
       else e = Calculer(equation, variable, b, NULL);

       tv1 = (d-f)/(a-c);
       tv2 = (e-f)/(b-c);

       if (d >= CODE_ERREUR)
            return d;
       else if (e >= CODE_ERREUR)
            return e;
       else if (f >= CODE_ERREUR)
            return f;

       if (VAbsolue((tv1-tv2)/(d-e)) > TAUX_VARIATION_DIFF_MAX)
           return 0;
       else return 1;
}


int ConfigEquaParam(char equation[], char variable)
{
    //permet de réécrire une fonction paramétrée en exprimant tout en fonction des deux fonctions de base
    //par ex, "cos((t^2, t/2))" devient "(cos(t^2), cos(t/2))"

    if (!equation)
       return ERR_PARAM;

    char *position = strchr(equation, ','),
         fonctionPartie1[MAX_CHAINE] = "",
         fonctionPartie2[MAX_CHAINE] = "",
         equation1[MAX_CHAINE] = "",
         equation2[MAX_CHAINE] = "";
    int nbParentheses , sortie = 0, i;

    while (position && !sortie)
    {
          nbParentheses = 0;
          for (i = position-equation ; i >= 0 && nbParentheses >= 0 ; i--)
          {
              if (equation[i] == ')')
                 nbParentheses++;
              else if (equation[i] == '(')
                   nbParentheses--;
          }
          i++;

          if (nbParentheses<0 && (i<1 || !TestLettre(equation[i-1]) || TestMultImp(equation, i, variable)))
             sortie = 1;
          else position = strchr(position+1, ',');
    }

    if (sortie)
    {
        strcpy(equation1, (char*)(i+(int)equation+1));
        equation1[position-equation-i-1] = '\0';

        strcpy(equation2, position+1);
        char *pos = RechercheParenthese(equation2);
        if (pos)
           *pos = '\0';

        strcpy(fonctionPartie1, equation);
        fonctionPartie1[i] = '\0';

        if (pos)
           strcpy(fonctionPartie2, pos+1);

        if (fonctionPartie1[0] || fonctionPartie2[0])
           sprintf(equation, "(%s(%s)%s,%s(%s)%s)", fonctionPartie1, equation1, fonctionPartie2, fonctionPartie1, equation2, fonctionPartie2);
        return 1;
    }
    else return 0;
}


int ExpliciterExpression(char expression[], int taille, char variable, Fonction *tab[], Variable *tab2[], int tailleTab, int tailleTab2)
{
    int EE(char expression[], int taille, char variable, Fonction *tab[], Variable *tab2[], int tailleTab, int tailleTab2, int reserve)
    {
        //fonction récursive
        //reserve doit être 0 !!

        reserve++;
        if (reserve > MAX_RECURSION)
           return ERR_RECURSION;
        if (!tab)
        {
           tab = RecupererTabFns();
           tailleTab = MAX_FONCTIONS;
        }
        if (!tab2)
        {
           tab2 = RecupererTabVar();
           tailleTab2 = MAX_VARIABLES;
        }

        if (!expression || taille <= 0)
           return ERR_PARAM;

        int i ,longueur;
        char *position,
             fonction[MAX_CHAINE] = "",
             partie1[MAX_CHAINE] = "",
             partie2[MAX_CHAINE] = "",
             partie3[MAX_CHAINE] = "",
             buffer[10] = "",
             nom[30] = "";

        for (i = 0 ; i < tailleTab && tab[i] ; i++)
        {
            sprintf(nom, "%s(", tab[i]->nom);

            if ((position = strstr(expression, nom)) && (position==expression || TestOperateurEx(*(position-1))))
            {
               strncpy(partie1, expression, MAX_CHAINE);
               partie1[position-expression] = '\0';

               if (!(position = strchr(position, '(')))
                  return ERR_SYNTAXE;
               strncpy(partie2, position, MAX_CHAINE-1);
               strcat(partie2, ")");

               if (!(position = RechercheParenthese(position+1)))
                  return ERR_SYNTAXE;
               strncpy(partie3, position+1, MAX_CHAINE);

               if (!(position = RechercheParenthese(partie2+1)))
                  return ERR_SYNTAXE;
               *(position+1) = '\0';

               strcpy(fonction, tab[i]->expression);
               buffer[0] = tab[i]->variable;
               RemplacerExpression(fonction, buffer, partie2, MAX_CHAINE);

               if (strlen(partie1)+strlen(fonction)+strlen(partie3) >= taille)
                  return ERR_TROPLONG;
               sprintf(expression, "%s(%s)%s", partie1, fonction, partie3);
               return EE(expression, taille, variable, tab, tab2, tailleTab, tailleTab2, reserve);
            }
        }
        for (i = 0 ; i < tailleTab2 && tab2[i] ; i++)
        {
            longueur = strlen(tab2[i]->nom);
            if ((position = strstr(expression, tab2[i]->nom)) && (position==expression || TestOperateurEx(*(position-1))) && (!(*(position+longueur)) || TestOperateurEx(*(position+longueur))))
            {
               strncpy(partie1, expression, MAX_CHAINE);
               partie1[position-expression] = '\0';
               strncpy(partie2, position+strlen(tab2[i]->nom), MAX_CHAINE);

               if (strlen(partie1)+strlen(tab2[i]->expression)+strlen(partie2)+2 >= taille)
                  return ERR_TROPLONG;
               sprintf(expression, "%s(%s)%s", partie1, tab2[i]->expression, partie2);
               return EE(expression, taille, variable, tab, tab2, tailleTab, tailleTab2, reserve);
            }
        }

        VariableDefaut *tabVariablesDefaut = RecupererTabVarDef();
        for (i = 0 ; tabVariablesDefaut[i].description[0] ; i++)
        {
            longueur = strlen(tabVariablesDefaut[i].nom);
            if ((position = strstr(expression, tabVariablesDefaut[i].nom)) && (position==expression || TestOperateurEx(*(position-1))) && (!(*(position+longueur)) || TestOperateurEx(*(position+longueur))))
            {
               strncpy(partie1, expression, MAX_CHAINE);
               partie1[position-expression] = '\0';
               strncpy(partie2, position+strlen(tabVariablesDefaut[i].nom), MAX_CHAINE);

               if (strlen(partie1)+strlen(tabVariablesDefaut[i].expression)+strlen(partie2)+2 >= taille)
                  return ERR_TROPLONG;

               if (tabVariablesDefaut[i].expression[0])
                  sprintf(expression, "%s(%s)%s", partie1, tabVariablesDefaut[i].expression, partie2);
               else sprintf(expression, "%s(%f)%s", partie1, tabVariablesDefaut[i].valeur, partie2);

               return EE(expression, taille, variable, tab, tab2, tailleTab, tailleTab2, reserve);
            }
        }

        return 1;
    }

    EnleverMultImp(expression, variable);
    return EE(expression, taille, variable, tab, tab2, tailleTab, tailleTab2, 0);
}


int RemplacerExpression(char chaine[], char aRemplacer[], char remplacerPar[], int taille)
{
    if (!chaine || !aRemplacer || !remplacerPar || taille <= 0)
       return ERR_PARAM;

    char *position = chaine,
         partie1[MAX_CHAINE] = "",
         partie2[MAX_CHAINE] = "";
    int longueur = strlen(aRemplacer),
        longueur2 = strlen(remplacerPar);

    while ( (position = strstr(position, aRemplacer)) )
    {
          if (TestFonction(chaine, position-chaine) != 1 && TestVariable(chaine, position-chaine) != 1)
          {
             strncpy(partie1, chaine, MAX_CHAINE);
             partie1[position-chaine] = '\0';

             strncpy(partie2, position+longueur, MAX_CHAINE);

             if (strlen(partie1)+strlen(partie2)+longueur2 >= taille)
                return ERR_TROPLONG;
             sprintf(chaine, "%s%s%s", partie1, remplacerPar, partie2);

             position = chaine+strlen(partie1)+longueur2+1;
          }
          else position++;
    }

    return 1;
}

int RecupererEquasParam(char equation[], char equation1[], char equation2[], char variable, int taille)
{
    char copieEquation[MAX_CHAINE],
         *tabEquation[2] = {NULL};

    strncpy(copieEquation, equation, MAX_CHAINE);
    if (!ConfigEquaParam(copieEquation, variable))
       return 0;

    if (copieEquation[0] != '(' || copieEquation[strlen(copieEquation)-1] != ')')
       return 0;

    tabEquation[0] = malloc(MAX_CHAINE);
    tabEquation[1] = malloc(MAX_CHAINE);
    RecupererParametres(copieEquation, NULL, 2, 0, 0, tabEquation, MAX_CHAINE);

    strncpy(equation1, tabEquation[0], taille);
    strncpy(equation2, tabEquation[1], taille);

    free(tabEquation[0]);
    free(tabEquation[1]);
    return 1;
}

int EnleverMultImp(char expression[], char variable)
{
    if (!expression)
        return 0;

    int i;
    char buffer[MAX_CHAINE];

    for (i=1 ; expression[i] ; i++)
    {
        if (TestMultImp(expression, i, variable))
        {
            strcpy(buffer, expression+i);
            expression[i] = '#';
            strcpy(expression+i+1, buffer);
            i++;
        }
    }

    return 1;
}

int ResoudreEquation(char expression[], char inconnue, double debut, double fin)
{
    if (!expression)
        return 0;

    char *position = strchr(expression, '=');
    if (!position)
        return 0;

    if (debut==fin)
        return 0;

    if (debut<fin)
    {
        double tmp = fin;
        fin = debut;
        debut = tmp;
    }

    int l1 = strlen(expression)+2,
        l2 = strlen(position+1);

    if (l1<=0 || l2<=0)
        return 0;

    char *e1 = malloc(l1+1);
    if (!e1)
        return 0;
    strncpy(e1, expression, l1);

    char *e2 = malloc(l2+1);
    if (!e2)
    {
        free(e1);
        return 0;
    }
    strncpy(e2, position+1, l2);

    strcat(e1, "-(");
    strcat(e1, e2);
    strcat(e1, ")");
    free(e2);

    double pas = (fin-debut)/1000.0, i, r,
           p = Calculer(e1, inconnue, debut, NULL),
           retour = CODE_ERREUR;
    int sortie = 0;

    for (i=debut+pas ; i<=fin && !sortie ; i+=pas)
    {
        r = Calculer(e1, inconnue, i, NULL);
        if (p<CODE_ERREUR && r<CODE_ERREUR && r*p <= 0)
            sortie = 1;
        else p = r;
    }

    if (!sortie)
        return ERR_SOLUTION;

    double y = i-pas,
           x = y-pas;

    if (p==0)
        retour = x;
    if (r==0)
        retour = y;

    int j;
    double m;
    sortie = 0;
    for (j=1 ; j<=20 && !sortie && retour == CODE_ERREUR ; j++)
    {
        m=Calculer(e1, inconnue, (x+y)/2.0, NULL);
        if (m==0)
            sortie=1;
        else if (m*p<0)
        {
            y=(x+y)/2.0;
            r=Calculer(e1, inconnue, y, NULL);
        }
        else
        {
            x=(x+y)/2.0;
            p=Calculer(e1, inconnue, x, NULL);
        }
    }

    if (retour == CODE_ERREUR)
        retour = (x+y)/2.0;

    free(e1);
    return retour;
}

