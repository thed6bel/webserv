#!/usr/bin/env python3

import cgi
import os

# Définissez le répertoire où vous souhaitez enregistrer les photos
UPLOAD_DIR = '/chemin/vers/votre/repertoire/photo'

# Créez un objet FieldStorage pour gérer les données du formulaire
form = cgi.FieldStorage()

# Vérifiez si le formulaire a été soumis et si le champ de fichier 'photo' existe
if 'photo' in form:
    # Récupérez le fichier de la photo
    photo = form['photo']
    # Vérifiez si le fichier a été correctement téléchargé
    if photo.filename:
        # Enregistrez le fichier de la photo dans le répertoire d'envoi
        with open(os.path.join(UPLOAD_DIR, photo.filename), 'wb') as f:
            f.write(photo.file.read())
        print('Content-Type: text/html')
        print()
        print('<html><body>')
        print('<h1>Photo envoyée avec succès!</h1>')
        print('</body></html>')
    else:
        print('Content-Type: text/html')
        print()
        print('<html><body>')
        print('<h1>Aucun fichier sélectionné!</h1>')
        print('</body></html>')
else:
    print('Content-Type: text/html')
    print()
    print('<html><body>')
    print('<h1>Aucune donnée envoyée!</h1>')
    print('</body></html>')
