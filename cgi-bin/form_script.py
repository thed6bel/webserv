#!/usr/bin/env python3

import cgi

# En-tête pour indiquer que le contenu est HTML
#print("Content-Type: text/html\n")
# while 1

# Récupérer les données du formulaire
form = cgi.FieldStorage()

# Récupérer les valeurs des champs "name" et "email"
name = form.getvalue("name")
email = form.getvalue("email")

# Afficher les données
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>Request from CGI</title>")
print("</head>")
print("<body>")
print("<h1>Data from CGI</h1>")
print("<p>Nom: {}</p>".format(name))
print("<p>Email: {}</p>".format(email))
print("<form><input type='button' value='Back' onclick='history.back()'></form>")
print("</body>")
print("</html>")