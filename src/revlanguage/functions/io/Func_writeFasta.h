/**
 * @file
 * This file contains the declaration of Func_writeFasta, which writes the content of a CharacterData object into
 * a file with fasta format.
 *
 * @brief Declaration of Func_writeFasta
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2012-05-27 16:10:36 +0200 (Sun, 27 May 2012) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 *
 * $Id: Func_readCharacterData.h 1587 2012-05-27 14:10:36Z hoehna $
 */

#ifndef Func_writeFasta_H
#define Func_writeFasta_H

#include "RlFunction.h"


namespace RevLanguage {
    
    class Func_writeFasta :  public Function {
        
    public:
        // Basic utility functions
        Func_writeFasta*            clone(void) const;                                                      //!< Clone the object
        static const std::string&   getClassName(void);                                                     //!< Get class name
        static const TypeSpec&      getClassTypeSpec(void);                                                 //!< Get class type spec
        const TypeSpec&             getTypeSpec(void) const;                                                //!< Get language type of the object
        
        // Regular functions
        RbLanguageObject*           execute(void);                                                          //!< Execute function
        const ArgumentRules&        getArgumentRules(void) const;                                           //!< Get argument rules
        const TypeSpec&             getReturnType(void) const;                                              //!< Get type of return value
        
        
    };
    
}

#endif
