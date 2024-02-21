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

rep cgi-bin ou mettre les cgi dans le meme rep que le html?

/!\ creer 
fd_set pour les descripteurs prêts en lecture. // OK
fd_set pour les descripteurs prêts en écriture. // pas fait
fd_set pour les descripteurs avec des exceptions. //pas fait

Setup routes in a server to different directories. //?? root dans la config ou root dans location?
Setup a default file to search for if you ask for a directory. //pas compris
For every test you should receive the appropriate status code. //?

/!\
You can use a script containing an infinite loop or an error; you are free to do whatever tests you want within the limits of acceptability that remain at your discretion. The group being evaluated should help you with this.
// verifier l'exec du script

fonctionne plus :
// Si le chemin est vide, charge la page d'accueil (index.html, voir plus tard le php et autre...)
//ne fonctionne pas car le site est setup dans plusieurs rep, l'indes n'est pas a la racine
                if (requestedPath.empty() || requestedPath == "/")
                    requestedPath = "/index.html";




faire le parsing sur le header recu 
get OK GET /site/5/index.html HTTP/1.1
POST KO POST /site/5/index.html HTTP/1.1
delete KO DELETE /site/5/hello.jpeg undefined
options KO OPTIONS /site/5/hello.jpeg HTTP/1.1

en cas de methode delete il faut envoyer une reponse header CORS qui ressemble a ca :
HTTP/1.1 200 OK
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS
Access-Control-Allow-Headers: Content-Type

et les methode allowed sont setup dans le fichier de conf!