Code Arduino de la lampe d'Agathe.

Une lampe avec 3 ampoules dans lesquelles on a inséré des DEL RGB.

Contrôles en entrée
 
 - un bouton pressoir (Digital - INPUT - Pin 4)
 - une résistance variable (Analog - INPUT - Pin A0)

Contrôles en sortie

 - Signal pour les DEL RGB (Digital - OUTPUT - Pin 7)

Configuration actuelle

1. Première pression du bouton => La rotation choisit la couleur
2. Deuxième pression du bouton => La ratation change aléatoirement les couleurs
3. Troisième pression du bonton => La rotation décale progressivement les couleurs
4. Quatrième pression du bouton => Cycle arc-en-ciel, la rotation du bouton détermine la vitesse du changement
