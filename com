j'ai codé un serveur web en C++98, il fonctionne mais il est configurer de manière très simple, 
maintenant je doit faire le parsing du fichier de configuration pour pouvoir envoyer les configurations au serveur

j'ai tester plusieurs manière de faire mais aucune ne fonctionne à 100%

fait moi le parsing du fichier de configuration pour qu'il puisse être envoyer au serveur web et le 
le fichier de configuration est envoyé en paramètre exemple ""./pars config.cfg" ou ""./pars test"
voici le contenu du fichier de configuration:
server {
    listen 8000;
    server_name localhost;
    host 127.0.0.1;
    root site/1/;
    index index.html;
    error_page 404 error_pages/404.html;

    location /site/1 {
        allow_methods DELETE POST GET;
        autoindex off;
    }
}

server {
    listen 8001;
    server_name localhost;
    host 127.0.0.1;
    root site/2/;
    index index.html;
    error_page 404 error_pages/404.html;

    location /site/2 {
        allow_methods DELETE POST GET;
        autoindex off;
    }
}

server {
    listen 8002;
    server_name localhost;
    host 127.0.0.1;
    root site/3/;
    index index.html;
    error_page 404 error_pages/404.html;

    location /site/3 {
        allow_methods DELETE POST GET;
        autoindex on;
        index hello.html;
    }

    location /tours {
        autoindex on;
        index tours1.html;
        allow_methods GET POST PUT HEAD;
    }

    location /hello_c {
        autoindex on;
        index hello_c.html;
        allow_methods GET POST PUT HEAD;
    }
}

le résultat afficher sur le terminale donnerait :

server 1
    listen : 8000
    server_name : localhost
    host : 127.0.0.1
    root : site/1/
    index : index.html
    error_page : 404 : error_pages/404.html

    location : /site/1
        allow_methods : DELETE POST GET
        autoindex : off


server 2
    listen : 8001
    server_name : localhost
    host : 127.0.0.1
    root : site/2/
    index : index.html
    error_page : 404 : error_pages/404.html

    location : /site/2
        allow_methods : DELETE POST GET
        autoindex : off

server 3
    listen : 8003
    server_name : localhost
    host : 127.0.0.1
    root : site/3/
    index : index.html
    error_page : 404 : error_pages/404.html
    location : /site/3
        allow_methods : DELETE POST GET
        autoindex : on
        index : hello.html
    location /tours 
        autoindex : on
        index : tours1.html
        allow_methods : GET POST PUT HEAD
    location /hello_c
        autoindex : on
        index : hello_c.html
        allow_methods : GET POST PUT HEAD