/*
server_name site perso;
affiche : Server Name: site
coucou
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
delete ok DELETE /site/5/hello.jpeg undefined
options ok OPTIONS /site/5/hello.jpeg HTTP/1.1

en cas de methode delete il faut envoyer une reponse header CORS qui ressemble a ca :
HTTP/1.1 200 OK
Access-Control-Allow-Origin: *
Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS
Access-Control-Allow-Headers: Content-Type

et les methode allowed sont setup dans le fichier de conf!

.PHP a gerer!!!!!!!
devrait pouvoir télécharger des fichiers
Votre serveur doit pouvoir écouter sur plusieurs ports

header multipart/form-data:
POST /site/5/index.html HTTP/1.1
Host: 127.0.0.1
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:108.0) Gecko/20100101 Firefox/108.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Content-Type: multipart/form-data; boundary=---------------------------297572814233298941811707765879
Content-Length: 12494
Origin: http://127.0.0.1
Connection: keep-alive
Referer: http://127.0.0.1/site/5/index.html
Upgrade-Insecure-Requests: 1
Sec-Fetch-Dest: document
Sec-Fetch-Mode: navigate
Sec-Fetch-Site: same-origin
Sec-Fetch-User: ?1

-----------------------------297572814233298941811707765879
Content-Disposition: form-data; name="photo"; filename="takumi-fujiwara.png"
Content-Type: image/png

�PNG

IHDRr�M7%KPLTE��Ϯ�����
                        ��ա������������Ѳé��¤������*!���C5-����{d�cPXerw��cK>��zCJR4�5m IDATx���v�6���:�H        �H���?��B]�$%v������:]�v6A�/��������Ɛ�rC��������C���`���hsyon �i        �`���()�} �s
��G�T���o
`�"l��8�;y�}0s��1�F�[X�����Ŕ~��N^�g���R ﰜ�邨�!t٢�����C�g����D��=�*�t@�_�H���7H}[���rV��Sw\�B�If�eu�rF��K�e�
                                                                                                           �ƈ8]|������|�N��ԢN�l��_{g��0
                                                                                                                                       D� l��J����&�I2�.Jm���>\���I�Lk����&���d'�,7��_2e��A�:s�Κ��-�,7�Xu���:r�fҫl]
                                                                                                                                                                                                                   �k�������br��z�nj;���H/%�&O�J�;mT
                                                                                                                                                                                                                                                    }ҭ��ou�q@r���@��`R��d�.sJʁ@u��
                                                                                                                                                                                                                                                                                 �|�)����<�\j�k���������}80�?�ovw�\O%��o�?�A�BF&5\��]� ߭�������k1��.��GO�t��Zw)�����]���V�\Kȏuy��غ<}���#2�:`VyY��A~��MлȈ�B
 r�v()��W��A>(��l!u�@>T�o!��;��C��ߡ�Tg��9�QArT��� 9`@rT���䐠ErT��� G9�A���m���D�IEND�B`�
-----------------------------297572814233298941811707765879--
*/
