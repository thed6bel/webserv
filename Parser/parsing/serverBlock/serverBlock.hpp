#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include "locationBlock.hpp"


class ServerBlock
{
    private:
        char    *listen;
        char    *serverName;
        char    *root;
        char    *index;
        char    *uploadFiles;
        char    *errorPage;
        int     maxBodySize;
    
    public:
        ServerBlock();
        ~ServerBlock();
        LocationBlock   *firstLocation;
        
        //------------Getters & Setters---------------
        void    setListen(const char *newListen);
        void    setServerName(const char *newServerName);
        void    setRoot(const char *newRoot);
        void    setIndex(const char *newIndex);
        void    setUploadFiles(const char *newUploadFiles);
        void    setErrorPage(const char *newErrorPage);
        void    setMaxBodySize(int newBodySize);

        const char    *getListen(void ) const;
        const char    *getServerName(void ) const;
        const char    *getRoot(void ) const;
        const char    *getIndex(void ) const;
        const char    *getUploadFiles(void ) const;
        const char    *getErrorPage(void ) const;
        int             getMaxBodySize(void ) const;
        //--------------------------------------------

        //-----------Linked List accessors------------
        LocationBlock   *lastLocation(void );
        //--------------------------------------------

        ServerBlock   *next;
} ;

#endif