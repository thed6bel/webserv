#ifndef LOCATIONBLOCK_HPP
#define LOCATIONBLOCK_HPP

#include <iostream>

class LocationBlock
{
    // TODO Loctaion "=" !! a.t.m. not assignable
    
    private:
        char    *URL;
        char    *index;
        char    *autoIndex;
        int     limitExcept;    //Allowed functions
    public:
        LocationBlock();
        ~LocationBlock();
        LocationBlock &operator=(const LocationBlock &other);

        //----------- Getters & Setters --------------
        void    setURL(const char *newURL);
        void    setIndex(const char *newIndex);
        void    setAutoIndex(const char *newAutoIndex);
        void    setLimitExcept(int newLimitExcept);

        const char    *getURL(void ) const;
        const char    *getIndex(void ) const;
        const char    *getAutoIndex(void ) const;
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