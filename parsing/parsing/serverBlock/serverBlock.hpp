#ifndef SERVERBLOCK_HPP
#define SERVERBLOCK_HPP

#include "locationBlock.hpp"
    //Linked list for error_page struct;

class ServerBlock
{
    struct s_errorPage
    {
        char    *errorCode;
        char    *indexErrorPage;
    };

    private:
        char    *listen;
        char    *serverName;
        char    *root;
        char    *index; // index.html home.html
        s_errorPage   errorPage;
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
        void    setErrorPage(const char *newErrorPage, const char *newErrorCode);
        void    setMaxBodySize(int newBodySize);

        const char    *getListen(void ) const;
        const char    *getServerName(void ) const;
        const char    *getRoot(void ) const;
        const char    *getIndex(void ) const;
        s_errorPage      getErrorPage(void ) const;
        int             getMaxBodySize(void ) const;
        //--------------------------------------------

        //-----------Linked List accessors------------
        LocationBlock   *lastLocation(void );
        //--------------------------------------------

        ServerBlock   *next;
} ;

#endif