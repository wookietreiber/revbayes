/**
 * @file
 * This file contains the declaration of Alignment, which is
 * class that holds a character matrix in RevBayes.
 *
 * @brief Declaration of Alignment
 *
 * (c) Copyright 2009-
 * @date Last modified: $Date$
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 *
 * $Id$
 */

#ifndef CharacterData_H
#define CharacterData_H

#include "CharacterState.h"
#include "TaxonData.h"

#include <set>
#include <string>
#include <vector>


class CharacterData {

    public:
                                            CharacterData(const std::string &t);                                        //!< Constructor requires character type
                                            CharacterData(const CharacterData& x);                                      //!< Copy constructor to deal with sequenceTypeRule
        virtual                            ~CharacterData(void);                                                        //!< Destructor to deal with sequenceTypeRule

        // Overloaded operators
        CharacterData&                      operator=(const CharacterData& x);                                          //!< Assignment operator
        const TaxonData&                    operator[](size_t i) const;                                                 //!< Subscript operator (const)
        bool                                operator==(const CharacterData& x) const;                                   //!< Less than operator
        bool                                operator!=(const CharacterData& x) const;                                   //!< Less than operator
        bool                                operator<(const CharacterData& x) const;                                    //!< Less than operator
    
        // Container functions
        void                                clear(void);
//        void                                setElement(const size_t index, RbLanguageObject* elem);                     //!< Set element with type conversion

        // Matrix functions
//        const RbObject&                     getElement(size_t row, size_t col) const;                                   //!< Get element or subcontainer
//        RbObject&                           getElement(size_t row, size_t col);                                         //!< Get element or subcontainer (non-const to return non-const element)
//        void                                setElement(size_t row, size_t col, RbLanguageObject* var);                  //!< set element
//        size_t                              size(void) const;
//        void                                resize(size_t nRows, size_t nCols);                                         //!< Resize to new length vector
//        void                                transpose(void);                                                            //!< Transpose the matrix

        // CharacterData functions
        void                                addTaxonData(const TaxonData &obs);                                         //!< Add taxon data
        void                                addTaxonData(const TaxonData &obs, bool forceAdd);                          //!< Add taxon data
        void                                excludeCharacter(size_t i);                                                 //!< Exclude character
        void                                excludeTaxon(size_t i);                                                     //!< Exclude taxon
        void                                excludeTaxon(std::string& s);                                               //!< Exclude taxon
        const CharacterState&               getCharacter(size_t tn, size_t cn) const;                                   //!< Return a reference to a character element in the character matrix
        const std::string&                  getDataType(void) const;                                                    //!< Returns the data type for the matrix
        const std::string&                  getFileName(void) const;                                                    //!< Returns the name of the file the data came from
        bool                                getIsHomologyEstablished(void) const { return isHomologyEstablished; }      //!< Returns whether the homology of the characters has been established
        size_t                              getNumberOfCharacters(void) const;                                          //!< Number of characters
        size_t                              getNumberOfCharacters(size_t idx) const;                                    //!< Number of characters for a specific taxon
        size_t                              getNumberOfStates(void) const;                                              //!< Get the number of states for the characters in this matrix
        size_t                              getNumberOfTaxa(void) const;                                                //!< Number of taxa
        TaxonData&                          getTaxonData(size_t tn);                                                    //!< Return a reference to a sequence in the character matrix
        const TaxonData&                    getTaxonData(size_t tn) const;                                              //!< Return a reference to a sequence in the character matrix
        const std::string&                  getTaxonNameWithIndex(size_t idx) const;                                    //!< Returns the idx-th taxon name
        bool                                isCharacterExcluded(size_t i) const;                                        //!< Is the character excluded
        bool                                isTaxonExcluded(size_t i) const;                                            //!< Is the taxon excluded
        bool                                isTaxonExcluded(std::string& s) const;                                      //!< Is the taxon excluded
//        RlVector<char, Character>*          makeSiteColumn(size_t cn) const;                                            //!< Return a reference to a sequence in the character matrix
        void                                restoreCharacter(size_t i);                                                 //!< Restore character
        void                                restoreTaxon(size_t i);                                                     //!< Restore taxon
        void                                restoreTaxon(std::string& s);                                               //!< Restore taxon
        void                                setFileName(const std::string fn) { fileName = fn; }                        //!< Set the file name
        void                                setIsHomologyEstablished(bool tf) { isHomologyEstablished = tf; }           //!< Set whether the homology of the characters has been established
//        void                                showData(void);                                                             //!< Show the data in the character matrix
    
    private:
        // Utility functions
        size_t                              indexOfTaxonWithName(std::string& s) const;                                 //!< Get the index of the taxon
        bool                                isCharacterConstant(size_t idx) const;                                      //!< Is the idx-th character a constant pattern?
        bool                                isCharacterMissingOrAmbiguous(size_t idx) const;                            //!< Does the character have missing or ambiguous data?
        size_t                              numConstantPatterns(void) const;                                            //!< The number of constant patterns
        size_t                              numMissAmbig(void) const;                                                   //!< The number of patterns with missing or ambiguous characters

        // Member variables
        std::string                         characterType;
        std::set<size_t>                    deletedTaxa;                                                                //!< Set of deleted taxa
        std::set<size_t>                    deletedCharacters;                                                          //!< Set of deleted characters
        std::string                         fileName;                                                                   //!< The path/filename from where this matrix originated
        std::vector<std::string>            sequenceNames;                                                              //!< names of the sequences
        size_t                              sequenceLength;                                                             //!< The length of each sequence
        bool                                isHomologyEstablished;                                                      //!< Whether the homology of the characters has been established
        
        std::map<std::string, TaxonData>    taxonMap;
};



#endif
