#!/usr/bin/perl

use strict;
use warnings;
use CGI;

# Crée un objet CGI pour récupérer les paramètres de requête
my $cgi = CGI->new();

# boucle infinie
# while (1) {

# }

# Récupère le paramètre 'name' de la requête
my $name_param = $cgi->param('name') || 'Anonymous POWER';

# Génère la réponse HTML
#print "<html><body><h1>Hello, $name_param!</h1><a href="index.html"><button>Home Page</button></a></body></html>";
print "<!DOCTYPE html><html><body><h1>Hello, $name_param!</h1><a href=\"/index.html\"><button>Home Page</button></a></body></html>";
