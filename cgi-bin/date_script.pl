#!/usr/bin/perl

use strict;
use warnings;

# boucle infinie pour le test :
# while (1) {
# }

# Obtenir la date et l'heure actuelles
my $current_datetime = localtime;

# Afficher les données
print <<HTML;
<!DOCTYPE html>
<html>
<head>
<title>Hour and date</title>
</head>
<body>
<h1>Hour and date actualy</h1>
<p>$current_datetime</p>
<button onclick="goBack()">Back</button>

<script>
function goBack() {
  window.history.back();
}
</script>
</body>
</html>
HTML
