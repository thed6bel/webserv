server_name site perso;
affiche : Server Name: site

return /tours;


    location /script {
		root ./;  //ajouter mais part en couille...
        allow_methods GET POST DELETE;
        index time.py;
        cgi_path /usr/bin/python3 /bin/bash;//utile? possibilite de les ajouter dans le code php
        cgi_ext .py .sh;//utile? possibilite de les ajouter dans le code php
	}

