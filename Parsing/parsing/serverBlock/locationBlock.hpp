#ifndef LOCATIONBLOCK_HPP
#define LOCATIONBLOCK_HPP

#include <iostream>

class LocationBlock
{
    private:
        char    *URL;
        char    *index;
        char    *autoIndex;
        char    *returnPage;
        int     limitExcept;
    public:
        LocationBlock();
        ~LocationBlock();
        LocationBlock &operator=(const LocationBlock &other);

        //----------- Getters & Setters --------------
        void    setURL(const char *newURL);
        void    setIndex(const char *newIndex);
        void    setAutoIndex(const char *newAutoIndex);
        void    setReturnPage(const char *newReturnPage);
        void    setLimitExcept(int newLimitExcept);

        const char    *getURL(void ) const;
        const char    *getIndex(void ) const;
        const char    *getAutoIndex(void ) const;
        const char    *getReturnPage(void ) const;
        int getLimitExcept(void ) const;
        //--------------------------------------------
        
        //------------ Method Permissions ------------
        int getAllow(void );
        int postAllow(void );
        int deleteAllow(void );
        int optionAllow(void );
        //--------------------------------------------

        LocationBlock *next;

};

#endif